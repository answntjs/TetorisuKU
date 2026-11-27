#pragma once

#include "_PursuitStage.h"
#include <random>

inline constexpr float FearRiseDelay = 0.7f;

struct GimmickUpdate
{
	int scorePenalty = 0;
	PursuitCue cue = PursuitCue::None;
};

class StoryGimmick
{
public:
	virtual ~StoryGimmick() = default;
	virtual PursuitCue activate(Player& player, std::mt19937& rng) = 0;
	virtual GimmickUpdate update(float, Player&, int) { return {}; }
	virtual float scoreLockRemaining() const { return 0.0f; }
};

std::unique_ptr<StoryGimmick> makeStoryGimmick(StoryGimmickKind kind);
