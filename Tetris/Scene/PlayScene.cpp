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
}

void PlayScene::handleInput(const InputHandler& input)
{
	if (input.isKeyPressed(VK_ESCAPE))
	{
		game_.pushScene(std::make_unique<PauseScene>(game_, mode_, startLevel_, borderless_));
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
			const Player* p0 = gameManager_.getPlayer(0);
			const int score = p0 ? p0->getScore() : 0;
			const int lines = p0 ? p0->getTotalClearedLines() : 0;
			game_.pushScene(std::make_unique<GameOverScene>(
				game_, gameManager_.getEndMessage(), score, lines,
				mode_, startLevel_, borderless_));
		}
		return;
	}

	if (lastInput_)
	{
		gameManager_.update(dt, *lastInput_);
		lastInput_ = nullptr;
	}
}

void PlayScene::render(Console& console, const Renderer& renderer) const
{
	renderer.renderGame(console, gameManager_);
}
