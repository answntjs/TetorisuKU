#include "_Game.h"
#include "../Scene/_TitleScene.h"
#include <algorithm>
#include <chrono>

Game::Game(std::ostream& out)
	: console_(out)
{
	pushScene(std::make_unique<TitleScene>(*this));
	applyPendingTransitions();
}

void Game::run()
{
	using clock = std::chrono::steady_clock;
	auto prevTime = clock::now();

	while (running_ && !sceneStack_.empty())
	{
		const auto now = clock::now();
		float dt = std::chrono::duration<float>(now - prevTime).count();
		prevTime = now;
		dt = std::min(dt, 0.05f);

		input_.update(dt);

		if (!sceneStack_.empty())
		{
			sceneStack_.back()->handleInput(input_);
		}
		if (!sceneStack_.empty())
		{
			sceneStack_.back()->update(dt);
		}

		applyPendingTransitions();
		if (!running_ || sceneStack_.empty())
		{
			break;
		}

		console_.clear(Color::Black);

		// Render from lowest non-transparent scene upwards
		int startIdx = static_cast<int>(sceneStack_.size()) - 1;
		while (startIdx > 0 && sceneStack_[startIdx]->isTransparent())
		{
			--startIdx;
		}
		for (int i = startIdx; i < static_cast<int>(sceneStack_.size()); ++i)
		{
			sceneStack_[i]->render(console_, renderer_);
		}

		console_.present();
		Sleep(10);
	}
}

void Game::quit()
{
	running_ = false;
}

void Game::pushScene(std::unique_ptr<Scene> scene)
{
	pendingTransitions_.push_back({ TransitionType::Push, std::move(scene) });
}

void Game::popScene()
{
	pendingTransitions_.push_back({ TransitionType::Pop, nullptr });
}

void Game::changeScene(std::unique_ptr<Scene> scene)
{
	pendingTransitions_.push_back({ TransitionType::Change, std::move(scene) });
}

void Game::applyPendingTransitions()
{
	for (auto& tr : pendingTransitions_)
	{
		if (tr.type == TransitionType::Push)
		{
			if (tr.scene)
			{
				sceneStack_.push_back(std::move(tr.scene));
				sceneStack_.back()->onEnter();
			}
		}
		else if (tr.type == TransitionType::Pop)
		{
			if (!sceneStack_.empty())
			{
				sceneStack_.back()->onExit();
				sceneStack_.pop_back();
			}
		}
		else if (tr.type == TransitionType::Change)
		{
			while (!sceneStack_.empty())
			{
				sceneStack_.back()->onExit();
				sceneStack_.pop_back();
			}
			if (tr.scene)
			{
				sceneStack_.push_back(std::move(tr.scene));
				sceneStack_.back()->onEnter();
			}
		}
	}
	pendingTransitions_.clear();
}
