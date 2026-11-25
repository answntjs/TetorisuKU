#pragma once

#include "_Scene.h"
#include "../Core/_Types.h"
#include <string>

class GameOverScene : public Scene
{
public:
	GameOverScene(Game& game, const std::string& message, int score, int lines,
		GameMode mode, int startLevel, bool borderless);

	void handleInput(const InputHandler& input) override;
	void update(float dt) override;
	void render(Console& console, const Renderer& renderer) const override;

	bool isTransparent() const override { return true; }

private:
	std::string message_;
	int score_;
	int lines_;
	GameMode mode_;
	int startLevel_;
	bool borderless_;
};
