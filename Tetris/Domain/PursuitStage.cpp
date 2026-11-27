#include "_PursuitStage.h"
#include "_StoryGimmick.h"
#include "_Player.h"
#include "../Core/_GameManager.h"
#include <algorithm>
#include <random>

namespace
{
	constexpr int kSoloGoalGap = 300;
	constexpr int kDuelTarget = 3000;
	constexpr float kNoticeDuration = 2.0f;

	constexpr std::array<StoryCase, StoryCaseCount> kCases = { {
		{ "경범죄", 650, 20.0f, 25, 0.60f, 0, { 0, 0, 0, 0, 0 } },
		{ "소매치기", 700, 18.0f, 30, 0.55f, 14, { 100, 0, 0, 0, 0 } },
		{ "사기", 750, 16.0f, 35, 0.48f, 17, { 0, 100, 0, 0, 0 } },
		{ "강도", 800, 16.0f, 40, 0.44f, 10, { 0, 0, 100, 0, 0 } },
		{ "살인 사건", 850, 15.0f, 45, 0.38f, 20, { 20, 25, 10, 30, 15 } }
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
					gimmicks_[i] = makeStoryGimmick(static_cast<StoryGimmickKind>(i));
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
			publish(gimmicks_[choose(rng_)]->activate(player, rng_));
		}

		void onUpdate(float dt, GameManager& game) override
		{
			auto& player = *game.getPlayer(0);
			status_.hostageTime = scoreLockRemaining();
			if (!player.isAlive() || isCleared(game))
			{
				if (status_.fear.cue == PursuitCue::FearWarning)
				{
					status_.fear = {};
				}
				return;
			}

			status_.notice.timeLeft = std::max(0.0f, status_.notice.timeLeft - dt);
			status_.fear.timeLeft = std::max(0.0f, status_.fear.timeLeft - dt);
			escapeTimer_ += dt;
			while (escapeTimer_ >= settings_.escapeInterval)
			{
				escapeTimer_ -= settings_.escapeInterval;
				addScore(-settings_.escapePenalty);
			}

			for (auto& gimmick : gimmicks_)
			{
				if (status_.score == 0 || !player.isAlive())
				{
					break;
				}
				if (gimmick)
				{
					const auto result = gimmick->update(dt, player, status_.target);
					addScore(-result.scorePenalty);
					publish(result.cue);
				}
			}
			if (status_.score == 0 && status_.fear.cue == PursuitCue::FearWarning)
			{
				status_.fear = {};
			}
			status_.hostageTime = scoreLockRemaining();
		}

	private:
		void publish(PursuitCue cue)
		{
			switch (cue)
			{
			case PursuitCue::None:
				break;
			case PursuitCue::FearWarning:
				status_.fear = { cue, FearRiseDelay };
				break;
			case PursuitCue::FearRise:
				status_.fear = { cue, kNoticeDuration };
				break;
			default:
				status_.notice = { cue, kNoticeDuration };
				break;
			}
		}

		float scoreLockRemaining() const
		{
			float remaining = 0.0f;
			for (const auto& gimmick : gimmicks_)
			{
				if (gimmick)
				{
					remaining = std::max(remaining, gimmick->scoreLockRemaining());
				}
			}
			return remaining;
		}

		const StoryCase& settings_;
		std::mt19937 rng_;
		std::array<std::unique_ptr<StoryGimmick>, StoryGimmickCount> gimmicks_;
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
