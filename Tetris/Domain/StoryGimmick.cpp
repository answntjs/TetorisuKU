#include "_StoryGimmick.h"
#include "_Player.h"
#include <cassert>
#include <utility>

namespace
{
	constexpr float kHostageLimit = 15.0f;

	class HoldTheft final : public StoryGimmick
	{
		PursuitCue activate(Player& player, std::mt19937&) override
		{
			return player.stealHoldBlock() ? PursuitCue::HoldStolen : PursuitCue::None;
		}
	};

	class FalsePreview final : public StoryGimmick
	{
		PursuitCue activate(Player& player, std::mt19937& rng) override
		{
			return player.fakeNextPreview(rng) ? PursuitCue::FalsePreview : PursuitCue::None;
		}
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
		PursuitCue activate(Player& player, std::mt19937& rng) override
		{
			if (marker_)
			{
				return PursuitCue::None;
			}
			auto marker = std::make_shared<HostageMarker>();
			if (!player.markHostage(marker, rng))
			{
				return PursuitCue::None;
			}
			marker_ = std::move(marker);
			remaining_ = kHostageLimit;
			return PursuitCue::HostageTaken;
		}

		GimmickUpdate update(float dt, Player& player, int target) override
		{
			if (!marker_)
			{
				return {};
			}
			if (marker_->wasRescued())
			{
				marker_.reset();
				player.setBanner("인질 구출!");
				return {};
			}
			remaining_ -= dt;
			if (remaining_ > 0.0f)
			{
				return {};
			}
			player.removeHostageMarker(*marker_);
			marker_.reset();
			return { target / 5, PursuitCue::HostageExpired };
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
		PursuitCue activate(Player& player, std::mt19937&) override
		{
			return player.lockCurrentRotation() ? PursuitCue::RotationLocked : PursuitCue::None;
		}
	};

	class RisingLayer final : public StoryGimmick
	{
		PursuitCue activate(Player&, std::mt19937&) override
		{
			if (remaining_ > 0.0f)
			{
				return PursuitCue::None;
			}
			remaining_ = FearRiseDelay;
			return PursuitCue::FearWarning;
		}

		GimmickUpdate update(float dt, Player& player, int) override
		{
			if (remaining_ <= 0.0f)
			{
				return {};
			}
			remaining_ -= dt;
			if (remaining_ > 0.0f)
			{
				return {};
			}
			player.receivePermanentLine();
			return { 0, PursuitCue::FearRise };
		}

		float remaining_ = 0.0f;
	};
}

std::unique_ptr<StoryGimmick> makeStoryGimmick(StoryGimmickKind kind)
{
	switch (kind)
	{
	case StoryGimmickKind::HoldTheft: return std::make_unique<HoldTheft>();
	case StoryGimmickKind::FalsePreview: return std::make_unique<FalsePreview>();
	case StoryGimmickKind::Hostage: return std::make_unique<HostageRescue>();
	case StoryGimmickKind::RotationLock: return std::make_unique<RotationLock>();
	case StoryGimmickKind::PermanentLine: return std::make_unique<RisingLayer>();
	case StoryGimmickKind::Count: break;
	}
	assert(false);
	return {};
}
