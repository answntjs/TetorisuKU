#include "_Stage.h"
#include "../Core/_GameManager.h"
#include "_Player.h"
#include <algorithm>
#include <array>

Stage::Stage(const StageConfig& config)
	: config_(config)
{
}

void Stage::onStart(GameManager&)
{
	garbageTimer_ = 0.0f;
}

void Stage::onUpdate(float dt, GameManager& gm)
{
	if (config_.garbageIntervalSec <= 0.0f)
	{
		return;
	}

	garbageTimer_ += dt;
	if (garbageTimer_ >= config_.garbageIntervalSec)
	{
		garbageTimer_ -= config_.garbageIntervalSec;
		for (size_t i = 0; i < gm.getPlayerCount(); ++i)
		{
			gm.sendGarbageToPlayer(static_cast<int>(i), config_.garbageLines, config_.garbageHoles);
		}
	}
}

void Stage::onLineClear(Player&, const LineClearResult&, GameManager&)
{
}

bool Stage::isCleared(const GameManager& gm) const
{
	if (config_.clearLineGoal <= 0 || gm.getPlayerCount() == 0)
	{
		return false;
	}
	return gm.getPlayer(0)->getStageClearedLines() >= config_.clearLineGoal;
}

StageConfig Stage::createOriginalStage(int stageNumber, bool borderless)
{
	struct OriginalData
	{
		int speedTicks;
		int stickRate;
		int clearLine;
	};

	constexpr std::array<OriginalData, 8> kStages = { {
		{ 40, 20, 20 },
		{ 38, 18, 20 },
		{ 35, 18, 20 },
		{ 30, 17, 20 },
		{ 25, 16, 20 },
		{ 20, 14, 20 },
		{ 15, 14, 20 },
		{ 10, 13,  0 }
	} };

	const int idx = std::clamp(stageNumber - 1, 0, static_cast<int>(kStages.size()) - 1);
	const auto& data = kStages[idx];

	StageConfig cfg;
	cfg.stageNumber = idx + 1;
	cfg.dropIntervalSec = static_cast<float>(data.speedTicks) * 0.015f;
	cfg.clearLineGoal = data.clearLine;
	cfg.stickRate = -1; // 기본 7-Bag 사용 (원본 확률 모드 필요 시 data.stickRate 사용 가능)
	cfg.borderless = borderless;
	cfg.name = "스테이지 " + std::to_string(cfg.stageNumber);
	return cfg;
}

StageConfig Stage::createPvPStage(bool borderless)
{
	StageConfig cfg;
	cfg.stageNumber = 1;
	cfg.dropIntervalSec = 0.55f;
	cfg.clearLineGoal = 0;
	cfg.borderless = borderless;
	cfg.name = "대전 모드";
	return cfg;
}

StageConfig Stage::createPvEStage(bool borderless)
{
	StageConfig cfg;
	cfg.stageNumber = 1;
	cfg.dropIntervalSec = 0.50f;
	cfg.clearLineGoal = 30;
	cfg.borderless = borderless;
	cfg.garbageIntervalSec = 7.0f;
	cfg.garbageLines = 1;
	cfg.garbageHoles = 2;
	cfg.name = "방해 줄 생존";
	return cfg;
}
