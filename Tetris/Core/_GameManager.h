#pragma once

#include "../Domain/_BotPlayer.h"
#include "../IO/_InputHandler.h"
#include "../Domain/_Player.h"
#include "../Domain/_Stage.h"
#include "_Types.h"
#include <memory>
#include <random>
#include <string>
#include <vector>

class GameManager
{
public:
	GameManager() = default;

	void setup(GameMode mode, int startLevel = 1, bool borderless = false,
		unsigned int seed = std::random_device{}());
	void update(float dt, const InputHandler& input);

	void sendGarbageToPlayer(int playerIndex, int nLines, int xHoles);
	void sendGarbageToOpponents(int attackerIndex, int nLines, int xHoles);

	GameMode getMode() const { return mode_; }
	int getStartLevel() const { return startLevel_; }
	bool isBorderless() const { return borderless_; }
	bool isFinished() const { return finished_; }
	bool isVictory() const { return victory_; }
	int getWinnerIndex() const { return winnerIndex_; }
	const std::string& getEndMessage() const { return endMessage_; }

	size_t getPlayerCount() const { return players_.size(); }
	const Player* getPlayer(size_t index) const;
	Player* getPlayer(size_t index);
	const Stage* getStage() const { return stage_.get(); }

private:
	void updateStoryProgress();
	void checkBoardWinner();
	void finishVersus(int winner);
	void finish(bool victory, std::string message, int winner = -1);
	int computeAttackLines(const LineClearResult& result) const;

	GameMode mode_ = GameMode::Single;
	int startLevel_ = 1;
	bool borderless_ = false;

	std::vector<std::unique_ptr<Player>> players_;
	std::unique_ptr<Stage> stage_;
	std::mt19937 storyRng_;

	bool finished_ = false;
	bool victory_ = false;
	int winnerIndex_ = -1;
	std::string endMessage_;
};
