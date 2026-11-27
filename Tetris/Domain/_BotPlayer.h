#pragma once

#include "_Player.h"

class BotPlayer : public Player
{
public:
	BotPlayer(const std::string& name, const Viewport& viewport,
		bool borderless = false, unsigned int seed = std::random_device{}(), int difficulty = 1);

	void reset(bool borderless, unsigned int seed) override;
	void update(float dt, const InputHandler& input, float dropInterval,
		LineClearResult& outResult, bool& outLocked) override;

private:
	void planBestMove();
	double evaluateBoard(const Board& simBoard, int clearedLines) const;
	float getCurrentActionInterval() const;

	int difficulty_ = 1;
	bool hasPlan_ = false;
	int targetAngle_ = 0;
	int targetX_ = 3;
	float actionTimer_ = 0.0f;
	float baseActionInterval_ = 0.42f;
	float elapsedTime_ = 0.0f;
};
