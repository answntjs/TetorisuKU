#include "_PlayScene.h"
#include "../Core/_Game.h"
#include "_GameOverScene.h"
#include "_PauseScene.h"

PlayScene::PlayScene(Game& game, GameMode mode, int startLevel, bool borderless)
	: Scene(game)
	, mode_(mode)
	, startLevel_(startLevel)
	, borderless_(borderless)
{
}

void PlayScene::onEnter()
{
	gameManager_.setup(mode_, startLevel_, borderless_);
	gameOverPushed_ = false;
	briefing_ = isStoryMode(mode_);
	promotion_ = false;
}

void PlayScene::handleInput(const InputHandler& input)
{
	if (input.isKeyPressed(VK_ESCAPE))
	{
		const int restartLevel = mode_ == GameMode::StorySolo
			? gameManager_.getStage()->getConfig().stageNumber : startLevel_;
		game_.pushScene(std::make_unique<PauseScene>(game_, mode_, restartLevel, borderless_));
		return;
	}

	if (briefing_)
	{
		if (input.isKeyPressed(VK_RETURN) || input.isKeyPressed(VK_SPACE))
		{
			briefing_ = false;
		}
		return;
	}

	if (!gameManager_.isFinished())
	{
		// Input for players is processed in update(dt) via GameManager::update(dt, input)
		lastInput_ = &input;
	}
}

void PlayScene::update(float dt)
{
	if (gameManager_.isFinished())
	{
		if (!gameOverPushed_)
		{
			gameOverPushed_ = true;
			game_.pushScene(std::make_unique<GameOverScene>(game_, gameManager_));
		}
		return;
	}

	if (lastInput_)
	{
		const int previousCase = gameManager_.getStage()->getConfig().stageNumber;
		gameManager_.update(dt, *lastInput_);
		lastInput_ = nullptr;
		if (mode_ == GameMode::StorySolo
			&& gameManager_.getStage()->getConfig().stageNumber != previousCase)
		{
			briefing_ = true;
			promotion_ = true;
		}
	}
}

void PlayScene::render(Console& console, const Renderer& renderer) const
{
	if (briefing_)
	{
		renderer.renderStoryBriefing(console, gameManager_, promotion_);
	}
	else
	{
		renderer.renderGame(console, gameManager_);
	}
}
