#pragma once

#include "_Scene.h"
#include "../Core/_Types.h"
#include <array>
#include <string>

class GameOverScene : public Scene
{
public:
	GameOverScene(Game& game, const GameManager& match);

	void handleInput(const InputHandler& input) override;
	void update(float dt) override;
	void render(Console& console, const Renderer& renderer) const override;

	bool isTransparent() const override { return !showsPromotion(); }

private:
	bool showsPromotion() const { return mode_ == GameMode::StorySolo && victory_; }

	std::string message_;
	std::array<std::string, 2> stats_;
	GameMode mode_;
	int restartLevel_;
	bool borderless_;
	int storyCase_;
	bool victory_;
	std::string pursuitSummary_;
};
