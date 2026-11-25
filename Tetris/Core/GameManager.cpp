#include "_GameManager.h"
#include <algorithm>
#include <random>

void GameManager::setup(GameMode mode, int startLevel, bool borderless)
{
	mode_ = mode;
	startLevel_ = std::clamp(startLevel, 1, 10);
	borderless_ = borderless;
	players_.clear();
	finished_ = false;
	victory_ = false;
	winnerIndex_ = -1;
	endMessage_.clear();

	const unsigned int sharedSeed = std::random_device{}();

	if (mode_ == GameMode::Single)
	{
		stage_ = std::make_unique<Stage>(Stage::createOriginalStage(startLevel_, borderless_));
		players_.push_back(std::make_unique<Player>(
			"PLAYER 1", KeyBinding::SingleDefault(), Viewport{ 36, 3 }, borderless_, sharedSeed));
	}
	else if (mode_ == GameMode::PvE)
	{
		stage_ = std::make_unique<Stage>(Stage::createPvEStage(borderless_));
		players_.push_back(std::make_unique<Player>(
			"PLAYER 1", KeyBinding::SingleDefault(), Viewport{ 36, 3 }, borderless_, sharedSeed));
	}
	else if (mode_ == GameMode::PvP)
	{
		stage_ = std::make_unique<Stage>(Stage::createPvPStage(borderless_));
		players_.push_back(std::make_unique<Player>(
			"PLAYER 1", KeyBinding::Player1Default(), Viewport{ 14, 3 }, borderless_, sharedSeed));
		players_.push_back(std::make_unique<Player>(
			"PLAYER 2", KeyBinding::Player2Default(), Viewport{ 72, 3 }, borderless_, sharedSeed));
	}
	else // GameMode::VsBot
	{
		stage_ = std::make_unique<Stage>(Stage::createPvPStage(borderless_));
		players_.push_back(std::make_unique<Player>(
			"PLAYER 1", KeyBinding::SingleDefault(), Viewport{ 14, 3 }, borderless_, sharedSeed));
		players_.push_back(std::make_unique<BotPlayer>(
			"BOT", Viewport{ 72, 3 }, borderless_, sharedSeed));
	}

	if (stage_)
	{
		stage_->onStart(*this);
	}
}

void GameManager::update(float dt, const InputHandler& input)
{
	if (finished_ || !stage_)
	{
		return;
	}

	const float dropInterval = stage_->getConfig().dropIntervalSec;

	for (size_t i = 0; i < players_.size(); ++i)
	{
		if (!players_[i]->isAlive())
		{
			continue;
		}

		LineClearResult result;
		bool locked = false;
		players_[i]->update(dt, input, dropInterval, result, locked);

		if (locked && result.clearedLines > 0)
		{
			for (const auto& effect : result.triggeredEffects)
			{
				if (effect)
				{
					effect->onDestroy(*players_[i], *this, 0, 0);
				}
			}

			stage_->onLineClear(*players_[i], result, *this);

			if (mode_ == GameMode::PvP || mode_ == GameMode::VsBot)
			{
				const int attack = computeAttackLines(result);
				if (attack > 0)
				{
					sendGarbageToOpponents(static_cast<int>(i), attack, 1);
				}
			}
		}
	}

	stage_->onUpdate(dt, *this);

	// Check stage progression or win/loss conditions
	if (mode_ == GameMode::Single)
	{
		if (!players_[0]->isAlive())
		{
			finished_ = true;
			victory_ = false;
			endMessage_ = "GAME OVER";
		}
		else if (stage_->isCleared(*this))
		{
			const int nextStageNum = stage_->getConfig().stageNumber + 1;
			if (nextStageNum <= 10)
			{
				stage_ = std::make_unique<Stage>(Stage::createOriginalStage(nextStageNum, borderless_));
				stage_->onStart(*this);
				players_[0]->resetStageClearedLines();
				players_[0]->setBanner("STAGE " + std::to_string(nextStageNum) + "!");
			}
			else
			{
				finished_ = true;
				victory_ = true;
				endMessage_ = "ALL STAGES CLEAR!";
			}
		}
	}
	else if (mode_ == GameMode::PvE)
	{
		if (!players_[0]->isAlive())
		{
			finished_ = true;
			victory_ = false;
			endMessage_ = "SURVIVAL FAILED";
		}
		else if (stage_->isCleared(*this))
		{
			finished_ = true;
			victory_ = true;
			endMessage_ = "SURVIVAL CLEAR!";
		}
	}
	else // PvP or VsBot
	{
		int aliveCount = 0;
		int lastAlive = -1;
		for (size_t i = 0; i < players_.size(); ++i)
		{
			if (players_[i]->isAlive())
			{
				++aliveCount;
				lastAlive = static_cast<int>(i);
			}
		}

		if (aliveCount <= 1)
		{
			finished_ = true;
			winnerIndex_ = lastAlive;
			victory_ = (winnerIndex_ == 0);
			if (winnerIndex_ >= 0)
			{
				endMessage_ = players_[winnerIndex_]->getName() + " WINS!";
			}
			else
			{
				endMessage_ = "DRAW!";
			}
		}
	}
}

void GameManager::sendGarbageToPlayer(int playerIndex, int nLines, int xHoles)
{
	if (playerIndex < 0 || playerIndex >= static_cast<int>(players_.size()))
	{
		return;
	}
	players_[playerIndex]->receiveGarbage(nLines, xHoles);
}

void GameManager::sendGarbageToOpponents(int attackerIndex, int nLines, int xHoles)
{
	for (size_t i = 0; i < players_.size(); ++i)
	{
		if (static_cast<int>(i) != attackerIndex && players_[i]->isAlive())
		{
			players_[i]->receiveGarbage(nLines, xHoles);
		}
	}
}

const Player* GameManager::getPlayer(size_t index) const
{
	if (index >= players_.size()) return nullptr;
	return players_[index].get();
}

Player* GameManager::getPlayer(size_t index)
{
	if (index >= players_.size()) return nullptr;
	return players_[index].get();
}

int GameManager::computeAttackLines(const LineClearResult& result) const
{
	int attack = 0;
	if (result.isTSpin)
	{
		attack = result.clearedLines * 2;
	}
	else
	{
		switch (result.clearedLines)
		{
		case 1: attack = 0; break;
		case 2: attack = 1; break;
		case 3: attack = 2; break;
		case 4: attack = 4; break;
		default: break;
		}
	}

	if (result.isBackToBack && result.clearedLines > 0)
	{
		attack += 1;
	}
	if (result.comboCount >= 2)
	{
		attack += std::min(result.comboCount - 1, 4);
	}
	return attack;
}
