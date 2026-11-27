#include "_GameManager.h"
#include "../Domain/_PursuitStage.h"
#include <algorithm>
#include <stdexcept>
#include <utility>
#include <vector>

void GameManager::setup(GameMode mode, int startLevel, bool borderless, unsigned int seed)
{
	mode_ = mode;
	const int maxLevel = mode == GameMode::StorySolo ? StoryCaseCount
		: mode == GameMode::VsBot ? 5 : 8;
	startLevel_ = std::clamp(startLevel, 1, maxLevel);
	borderless_ = borderless;
	players_.clear();
	finished_ = false;
	victory_ = false;
	winnerIndex_ = -1;
	endMessage_.clear();
	storyRng_.seed(seed ^ 0x517cc1b7u);

	switch (mode_)
	{
	case GameMode::Single:
		stage_ = std::make_unique<Stage>(Stage::createOriginalStage(startLevel_, borderless_));
		players_.push_back(std::make_unique<Player>(
			"플레이어 1", KeyBinding::SingleDefault(), Viewport{ 36, 3 }, borderless_, seed));
		break;
	case GameMode::PvE:
		stage_ = std::make_unique<Stage>(Stage::createPvEStage(borderless_));
		players_.push_back(std::make_unique<Player>(
			"플레이어 1", KeyBinding::SingleDefault(), Viewport{ 36, 3 }, borderless_, seed));
		break;
	case GameMode::PvP:
	case GameMode::StoryDuel:
	{
		const bool story = mode_ == GameMode::StoryDuel;
		const int top = story ? 9 : 3;
		stage_ = story ? makeStoryDuelStage(borderless_)
			: std::make_unique<Stage>(Stage::createPvPStage(borderless_));
		players_.push_back(std::make_unique<Player>(story ? "1P 경찰" : "플레이어 1",
			KeyBinding::Player1Default(), Viewport{ 14, top }, borderless_, seed));
		players_.push_back(std::make_unique<Player>(story ? "2P 도둑" : "플레이어 2",
			KeyBinding::Player2Default(), Viewport{ 72, top }, borderless_, seed));
		break;
	}
	case GameMode::VsBot:
		stage_ = std::make_unique<Stage>(Stage::createPvPStage(borderless_));
		players_.push_back(std::make_unique<Player>(
			"플레이어 1", KeyBinding::SingleDefault(), Viewport{ 14, 3 }, borderless_, seed));
		players_.push_back(std::make_unique<BotPlayer>(
			"봇 Lv." + std::to_string(startLevel_), Viewport{ 72, 3 }, borderless_, seed, startLevel_));
		break;
	case GameMode::StorySolo:
		stage_ = makeStorySoloStage(startLevel_, borderless_, storyRng_());
		players_.push_back(std::make_unique<Player>(
			"경찰", KeyBinding::SingleDefault(), Viewport{ 36, 10 }, borderless_, seed));
		break;
	default:
		throw std::invalid_argument("Unknown game mode");
	}

	stage_->onStart(*this);
}

void GameManager::update(float dt, const InputHandler& input)
{
	if (finished_ || !stage_)
	{
		return;
	}

	const float dropInterval = stage_->getConfig().dropIntervalSec;
	const bool versus = mode_ == GameMode::PvP || mode_ == GameMode::VsBot || mode_ == GameMode::StoryDuel;
	std::vector<std::pair<int, int>> pendingAttacks;

	for (size_t i = 0; i < players_.size(); ++i)
	{
		auto& player = *players_[i];
		if (!player.isAlive())
		{
			continue;
		}

		LineClearResult result;
		bool locked = false;
		player.update(dt, input, dropInterval, result, locked);
		if (!locked)
		{
			continue;
		}

		if (result.clearedLines > 0)
		{
			for (const auto& [effect, row, col] : result.triggeredEffects)
			{
				if (effect)
				{
					effect->onDestroy(player, *this, row, col);
				}
			}

			stage_->onLineClear(player, result, *this);
			if (versus)
			{
				const int attack = computeAttackLines(result);
				if (attack > 0)
				{
					pendingAttacks.emplace_back(static_cast<int>(i), attack);
				}
			}
		}

		stage_->onBlockLocked(player, result, *this);
	}

	for (const auto& [attackerIndex, attackLines] : pendingAttacks)
	{
		sendGarbageToOpponents(attackerIndex, attackLines, 1);
		stage_->onAttackSent(attackerIndex);
	}

	stage_->onUpdate(dt, *this);

	switch (mode_)
	{
	case GameMode::Single:
		if (!players_[0]->isAlive())
		{
			finish(false, "게임 오버");
		}
		else if (stage_->isCleared(*this))
		{
			const int next = stage_->getConfig().stageNumber + 1;
			stage_ = std::make_unique<Stage>(Stage::createOriginalStage(next, borderless_));
			stage_->onStart(*this);
			players_[0]->resetStageClearedLines();
			players_[0]->setBanner(next == 8 ? "스테이지 8 - 무한" : "스테이지 " + std::to_string(next) + "!");
		}
		break;
	case GameMode::PvE:
		if (!players_[0]->isAlive())
		{
			finish(false, "생존 실패");
		}
		else if (stage_->isCleared(*this))
		{
			finish(true, "생존 성공!");
		}
		break;
	case GameMode::StorySolo:
		updateStoryProgress();
		break;
	case GameMode::StoryDuel:
	{
		const auto& pursuit = *stage_->getPursuitStatus();
		if (pursuit.score >= pursuit.target)
		{
			finishVersus(0);
		}
		else if (pursuit.score == 0)
		{
			finishVersus(1);
		}
		else
		{
			checkBoardWinner();
		}
		break;
	}
	case GameMode::PvP:
	case GameMode::VsBot:
		checkBoardWinner();
		break;
	}
}

void GameManager::updateStoryProgress()
{
	if (!players_[0]->isAlive())
	{
		finish(false, "수사 실패 - 보드가 가득 찼다");
		return;
	}
	if (stage_->getPursuitStatus()->score == 0)
	{
		finish(false, "수사 실패 - 용의자가 달아났다");
		return;
	}
	if (!stage_->isCleared(*this))
	{
		return;
	}

	const int next = stage_->getConfig().stageNumber + 1;
	if (next > StoryCaseCount)
	{
		finish(true, "체포 성공 - 마지막 사건 해결!");
		return;
	}

	stage_ = makeStorySoloStage(next, borderless_, storyRng_());
	players_[0]->reset(borderless_, storyRng_());
	stage_->onStart(*this);
}

void GameManager::checkBoardWinner()
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
		finishVersus(lastAlive);
	}
}

void GameManager::finishVersus(int winner)
{
	if (mode_ == GameMode::StoryDuel)
	{
		const char* message = winner == 0 ? "체포 성공 - 경찰 승리"
			: winner == 1 ? "도주 성공 - 도둑 승리" : "무승부";
		finish(winner == 0, message, winner);
		return;
	}
	finish(winner == 0, winner >= 0 ? players_[winner]->getName() + " 승리!" : "무승부!", winner);
}

void GameManager::finish(bool victory, std::string message, int winner)
{
	finished_ = true;
	victory_ = victory;
	winnerIndex_ = winner;
	endMessage_ = std::move(message);
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
