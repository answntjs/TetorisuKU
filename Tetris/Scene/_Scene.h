#pragma once

#include "../IO/_Console.h"
#include "../IO/_InputHandler.h"
#include "../IO/_Renderer.h"

class Game;

class Scene
{
public:
	explicit Scene(Game& game) : game_(game) {}
	virtual ~Scene() = default;

	virtual void onEnter() {}
	virtual void onExit() {}
	virtual void handleInput(const InputHandler& input) = 0;
	virtual void update(float dt) = 0;
	virtual void render(Console& console, const Renderer& renderer) const = 0;

	virtual bool isTransparent() const { return false; }

protected:
	Game& game_;
};
