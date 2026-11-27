#pragma once

#include "_Stage.h"
#include <array>
#include <cstddef>
#include <memory>

enum class StoryGimmickKind { HoldTheft, FalsePreview, Hostage, RotationLock, PermanentLine, Count };
inline constexpr auto StoryGimmickCount = static_cast<std::size_t>(StoryGimmickKind::Count);

enum class PursuitCue
{
	None, HoldStolen, FalsePreview, HostageTaken, RotationLocked, FearWarning, FearRise, HostageExpired
};

struct PursuitMessage
{
	PursuitCue cue = PursuitCue::None;
	float timeLeft = 0.0f;
};

struct PursuitStatus
{
	int score = 0;
	int target = 0;
	float hostageTime = 0.0f;
	PursuitMessage notice;
	PursuitMessage fear;
	std::array<float, 2> attackTime{};
};

struct StoryCase
{
	const char* title;
	int startingScore;
	float escapeInterval;
	int escapePenalty;
	float dropInterval;
	int gimmickChance;
	std::array<int, StoryGimmickCount> gimmickWeights;
};

inline constexpr int StoryCaseCount = 5;
const StoryCase& getStoryCase(int caseNumber);
std::unique_ptr<Stage> makeStorySoloStage(int caseNumber, bool borderless, unsigned int seed);
std::unique_ptr<Stage> makeStoryDuelStage(bool borderless);
