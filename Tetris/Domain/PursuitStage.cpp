#include "_PursuitStage.h"
#include "_Player.h"
#include "../Core/_GameManager.h"
#include <algorithm>
#include <cassert>
#include <random>

namespace
{
	enum class GimmickKind { HoldTheft, FalsePreview, Hostage, RotationLock, PermanentLine };
	constexpr int kSoloGoalGap = 300;
	constexpr int kDuelTarget = 3000;
	constexpr float kHostageLimit = 15.0f;

	constexpr std::array<StoryCase, StoryCaseCount> kCases = { {
		{ "경범죄", "빨간 모자", "골목만 돌면 따돌릴 수 있어.",
			650, 20.0f, 25, 0.60f, 0, { 0, 0, 0, 0, 0 } },
		{ "소매치기", "은빛 손", "잠깐 한눈판 사이에도 손은 움직이지.",
			700, 18.0f, 30, 0.55f, 12, { 100, 0, 0, 0, 0 } },
		{ "사기", "가짜 신사", "다음 수가 보인다고? 눈을 믿어 봐.",
			750, 16.0f, 35, 0.48f, 15, { 0, 100, 0, 0, 0 } },
		{ "강도", "황금 가면", "인질부터 구해 봐. 시간이 얼마 없을걸.",
			800, 16.0f, 40, 0.44f, 8, { 0, 0, 100, 0, 0 } },
		{ "살인 사건", "검은 그림자", "이 골목 끝에서 기다리고 있지.",
			850, 15.0f, 45, 0.38f, 18, { 20, 25, 10, 30, 15 } }
	} };

	StageConfig soloConfig(int caseNumber, bool borderless)
	{
		StageConfig config;
		config.stageNumber = caseNumber;
		config.name = "사건 " + std::to_string(caseNumber) + "  " + getStoryCase(caseNumber).title;
		config.dropIntervalSec = getStoryCase(caseNumber).dropInterval;
		config.clearLineGoal = 0;
		config.borderless = borderless;
		return config;
	}

	StageConfig duelConfig(bool borderless)
	{
		auto config = Stage::createPvPStage(borderless);
		config.name = "2인 추격전";
		return config;
	}

	class StoryGimmick
	{
	public:
		virtual ~StoryGimmick() = default;
		virtual bool activate(Player& player, std::mt19937& rng) = 0;
		virtual std::string_view notice() const = 0;
		virtual int update(float, Player&, int) { return 0; }
		virtual float scoreLockRemaining() const { return 0.0f; }
	};

	class HoldTheft final : public StoryGimmick
	{
		bool activate(Player& player, std::mt19937&) override { return player.stealHoldBlock(); }
		std::string_view notice() const override { return "소매치기가 홀드 블록을 가져갔다!"; }
	};

	class FalsePreview final : public StoryGimmick
	{
		bool activate(Player& player, std::mt19937& rng) override { return player.fakeNextPreview(rng); }
		std::string_view notice() const override { return "다음 블록 정보가 수상하다!"; }
	};

	class HostageMarker final : public IBlockEffect
	{
	public:
		void onDestroy(Player&, GameManager&, int, int) override { rescued_ = true; }
		std::string getName() const override { return "HOSTAGE"; }
		Color getTintColor() const override { return Color::Yellow; }
		const char* getCellGlyph() const override { return "H!"; }
		bool wasRescued() const { return rescued_; }

	private:
		bool rescued_ = false;
	};

	class HostageRescue final : public StoryGimmick
	{
	public:
		bool activate(Player& player, std::mt19937& rng) override
		{
			if (marker_)
			{
				return false;
			}
			auto marker = std::make_shared<HostageMarker>();
			if (!player.markHostage(marker, rng))
			{
				return false;
			}
			marker_ = std::move(marker);
			remaining_ = kHostageLimit;
			return true;
		}

		std::string_view notice() const override { return "인질이 나타났다. H!가 있는 줄을 지워라!"; }

		int update(float dt, Player& player, int target) override
		{
			if (!marker_)
			{
				return 0;
			}
			if (marker_->wasRescued())
			{
				marker_.reset();
				player.setBanner("인질 구출!");
				return 0;
			}
			remaining_ -= dt;
			if (remaining_ > 0.0f)
			{
				return 0;
			}
			player.removeHostageMarker(*marker_);
			marker_.reset();
			return target / 5;
		}

		float scoreLockRemaining() const override
		{
			return marker_ && !marker_->wasRescued() ? remaining_ : 0.0f;
		}

	private:
		std::shared_ptr<HostageMarker> marker_;
		float remaining_ = 0.0f;
	};

	class RotationLock final : public StoryGimmick
	{
		bool activate(Player& player, std::mt19937&) override { return player.lockCurrentRotation(); }
		std::string_view notice() const override { return "현재 블록은 회전할 수 없다!"; }
	};

	class RisingLayer final : public StoryGimmick
	{
		bool activate(Player& player, std::mt19937&) override
		{
			player.receivePermanentLine();
			return true;
		}
		std::string_view notice() const override { return "삭제할 수 없는 층이 올라왔다!"; }
	};

	std::unique_ptr<StoryGimmick> makeGimmick(GimmickKind kind)
	{
		switch (kind)
		{
		case GimmickKind::HoldTheft: return std::make_unique<HoldTheft>();
		case GimmickKind::FalsePreview: return std::make_unique<FalsePreview>();
		case GimmickKind::Hostage: return std::make_unique<HostageRescue>();
		case GimmickKind::RotationLock: return std::make_unique<RotationLock>();
		case GimmickKind::PermanentLine: return std::make_unique<RisingLayer>();
		}
		assert(false);
		return {};
	}

	class PursuitStage : public Stage
	{
	public:
		const PursuitStatus* getPursuitStatus() const override { return &status_; }
		bool isCleared(const GameManager&) const override { return status_.score >= status_.target; }

	protected:
		PursuitStage(const StageConfig& config, int start, int target) : Stage(config)
		{
			status_.score = start;
			status_.target = target;
		}

		void addScore(int change)
		{
			status_.score = std::clamp(status_.score + change, 0, status_.target);
		}

		PursuitStatus status_;
	};

	class SoloPursuitStage final : public PursuitStage
	{
	public:
		SoloPursuitStage(int caseNumber, bool borderless, unsigned int seed)
			: PursuitStage(soloConfig(caseNumber, borderless), getStoryCase(caseNumber).startingScore,
				getStoryCase(caseNumber).startingScore + kSoloGoalGap)
			, settings_(getStoryCase(caseNumber))
			, rng_(seed)
		{
			for (size_t i = 0; i < gimmicks_.size(); ++i)
			{
				if (settings_.gimmickWeights[i] > 0)
				{
					gimmicks_[i] = makeGimmick(static_cast<GimmickKind>(i));
				}
			}
		}

		void onLineClear(Player&, const LineClearResult& result, GameManager&) override
		{
			if (scoreLockRemaining() <= 0.0f)
			{
				addScore(result.lineClearScore);
			}
		}

		void onBlockLocked(Player& player, const LineClearResult&, GameManager& game) override
		{
			if (!player.isAlive() || isCleared(game) || settings_.gimmickChance == 0
				|| !std::bernoulli_distribution(settings_.gimmickChance / 100.0)(rng_))
			{
				return;
			}

			std::discrete_distribution<size_t> choose(settings_.gimmickWeights.begin(),
				settings_.gimmickWeights.end());
			auto& gimmick = gimmicks_[choose(rng_)];
			if (gimmick->activate(player, rng_))
			{
				status_.notice = gimmick->notice();
				status_.noticeTime = 2.0f;
			}
		}

		void onUpdate(float dt, GameManager& game) override
		{
			auto& player = *game.getPlayer(0);
			status_.hostageTime = scoreLockRemaining();
			if (!player.isAlive() || isCleared(game))
			{
				return;
			}

			status_.noticeTime = std::max(0.0f, status_.noticeTime - dt);
			for (auto& gimmick : gimmicks_)
			{
				if (gimmick)
				{
					const int penalty = gimmick->update(dt, player, status_.target);
					if (penalty > 0)
					{
						addScore(-penalty);
						status_.notice = "구출 시간 초과. 추격 점수 감소!";
						status_.noticeTime = 2.0f;
					}
				}
			}
			status_.hostageTime = scoreLockRemaining();

			escapeTimer_ += dt;
			while (escapeTimer_ >= settings_.escapeInterval)
			{
				escapeTimer_ -= settings_.escapeInterval;
				addScore(-settings_.escapePenalty);
			}
		}

	private:
		float scoreLockRemaining() const
		{
			const auto& hostage = gimmicks_[static_cast<size_t>(GimmickKind::Hostage)];
			return hostage ? hostage->scoreLockRemaining() : 0.0f;
		}

		const StoryCase& settings_;
		std::mt19937 rng_;
		std::array<std::unique_ptr<StoryGimmick>, 5> gimmicks_;
		float escapeTimer_ = 0.0f;
	};

	class DuelPursuitStage final : public PursuitStage
	{
	public:
		explicit DuelPursuitStage(bool borderless)
			: PursuitStage(duelConfig(borderless), kDuelTarget / 2, kDuelTarget)
		{
		}

		void onLineClear(Player& player, const LineClearResult& result, GameManager& game) override
		{
			const int sign = &player == game.getPlayer(0) ? 1 : -1;
			frameScore_ += sign * result.lineClearScore;
		}

		void onAttackSent(int attacker) override
		{
			status_.attackTime[attacker] = 0.8f;
		}

		void onUpdate(float dt, GameManager&) override
		{
			addScore(frameScore_);
			frameScore_ = 0;
			for (float& duration : status_.attackTime)
			{
				duration = std::max(0.0f, duration - dt);
			}
		}

	private:
		int frameScore_ = 0;
	};
}

const StoryCase& getStoryCase(int caseNumber)
{
	return kCases[std::clamp(caseNumber, 1, StoryCaseCount) - 1];
}

std::unique_ptr<Stage> makeStorySoloStage(int caseNumber, bool borderless, unsigned int seed)
{
	return std::make_unique<SoloPursuitStage>(std::clamp(caseNumber, 1, StoryCaseCount), borderless, seed);
}

std::unique_ptr<Stage> makeStoryDuelStage(bool borderless)
{
	return std::make_unique<DuelPursuitStage>(borderless);
}
