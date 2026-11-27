#pragma once

#include "_Stage.h"
#include <array>
#include <memory>
#include <string_view>

struct PursuitStatus
{
	int score = 0;
	int target = 0;
	float hostageTime = 0.0f;
	std::string_view notice;
	float noticeTime = 0.0f;
	std::array<float, 2> attackTime{};
};

struct StoryCase
{
	const char* title;
	const char* suspect;
	const char* dialogue;
	int startingScore;
	float escapeInterval;
	int escapePenalty;
	float dropInterval;
	int gimmickChance;
	std::array<int, 5> gimmickWeights;
};

inline constexpr int StoryCaseCount = 5;
const StoryCase& getStoryCase(int caseNumber);
std::unique_ptr<Stage> makeStorySoloStage(int caseNumber, bool borderless, unsigned int seed);
std::unique_ptr<Stage> makeStoryDuelStage(bool borderless);
