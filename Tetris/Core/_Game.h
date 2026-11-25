#pragma once

#include "../IO/_Console.h"
#include "../IO/_InputHandler.h"
#include "../IO/_Renderer.h"
#include "../Scene/_Scene.h"
#include <iostream>
#include <memory>
#include <ostream>
#include <vector>

class Game
{
public:
	explicit Game(std::ostream& out = std::cout);
	~Game() = default;

	void run();
	void quit();

	void pushScene(std::unique_ptr<Scene> scene);
	void popScene();
	void changeScene(std::unique_ptr<Scene> scene);

private:
	enum class TransitionType
	{
		Push,
		Pop,
		Change
	};

	struct PendingTransition
	{
		TransitionType type;
		std::unique_ptr<Scene> scene;
	};

	void applyPendingTransitions();

	Console console_;
	InputHandler input_;
	Renderer renderer_;
	std::vector<std::unique_ptr<Scene>> sceneStack_;
	std::vector<PendingTransition> pendingTransitions_;
	bool running_ = true;
};
