#pragma once

#include "_Scene.h"
#include "../Core/_Types.h"

class PauseScene : public Scene
{
public:
	PauseScene(Game& game, GameMode mode, int startLevel, bool borderless);

	void handleInput(const InputHandler& input) override;
	void update(float dt) override;
	void render(Console& console, const Renderer& renderer) const override;

	bool isTransparent() const override { return true; }

private:
	GameMode mode_;
	int startLevel_;
	bool borderless_;
};
