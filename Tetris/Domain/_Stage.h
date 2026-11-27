#pragma once

#include "../Core/_Types.h"
#include <string>

class GameManager;
class Player;
struct PursuitStatus;

struct StageConfig
{
	int stageNumber = 1;
	float dropIntervalSec = 0.60f;
	int clearLineGoal = 20;
	int stickRate = -1;            // -1: 7-Bag, 0~99: 오리지널 막대 확률
	bool borderless = false;
	float garbageIntervalSec = 0.0f; // > 0이면 주기적으로 밑에서 방해 줄 상승
	int garbageLines = 1;
	int garbageHoles = 2;
	std::string name = "Stage 1";
};

class Stage
{
public:
	explicit Stage(const StageConfig& config = createOriginalStage(1));
	virtual ~Stage() = default;

	virtual void onStart(GameManager& gm);
	virtual void onUpdate(float dt, GameManager& gm);
	virtual void onLineClear(Player& player, const LineClearResult& result, GameManager& gm);
	virtual void onBlockLocked(Player&, const LineClearResult&, GameManager&) {}
	virtual void onAttackSent(int) {}
	virtual const PursuitStatus* getPursuitStatus() const { return nullptr; }
	virtual bool isCleared(const GameManager& gm) const;

	const StageConfig& getConfig() const { return config_; }
	void setBorderless(bool borderless) { config_.borderless = borderless; }
	float getGarbageTimer() const { return garbageTimer_; }

	static StageConfig createOriginalStage(int stageNumber, bool borderless = false);
	static StageConfig createPvPStage(bool borderless = false);
	static StageConfig createPvEStage(bool borderless = false);

protected:
	StageConfig config_;
	float garbageTimer_ = 0.0f;
};
