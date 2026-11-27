#pragma once

#include "../Core/_GameManager.h"
#include "_Scene.h"

class PlayScene : public Scene
{
public:
	PlayScene(Game& game, GameMode mode, int startLevel, bool borderless);

	void onEnter() override;
	void handleInput(const InputHandler& input) override;
	void update(float dt) override;
	void render(Console& console, const Renderer& renderer) const override;

private:
	GameMode mode_;
	int startLevel_;
	bool borderless_;
	GameManager gameManager_;
	const InputHandler* lastInput_ = nullptr;
	bool gameOverPushed_ = false;
	bool briefing_ = false;
	bool promotion_ = false;
};
