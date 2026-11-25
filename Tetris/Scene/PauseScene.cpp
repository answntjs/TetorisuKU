#include "_PauseScene.h"
#include "../Core/_Game.h"
#include "_PlayScene.h"
#include "../IO/_TextBox.h"
#include "_TitleScene.h"

PauseScene::PauseScene(Game& game, GameMode mode, int startLevel, bool borderless)
	: Scene(game)
	, mode_(mode)
	, startLevel_(startLevel)
	, borderless_(borderless)
{
}

void PauseScene::handleInput(const InputHandler& input)
{
	if (input.isKeyPressed(VK_ESCAPE) || input.isKeyPressed(VK_RETURN))
	{
		game_.popScene();
	}
	else if (input.isKeyPressed('R'))
	{
		game_.changeScene(std::make_unique<PlayScene>(game_, mode_, startLevel_, borderless_));
	}
	else if (input.isKeyPressed('T'))
	{
		game_.changeScene(std::make_unique<TitleScene>(game_));
	}
}

void PauseScene::update(float)
{
}

void PauseScene::render(Console& console, const Renderer&) const
{
	TextBox box(40, 12, 34,
		"Game is paused.\n\n"
		"  [ESC / ENTER] Resume\n"
		"  [R]           Restart\n"
		"  [T]           Title Menu",
		BorderStyle::Bold);
	box.setTitle("PAUSED");
	box.setColors(Color::White, Color::Yellow, Color::DarkBlue, true);
	box.draw(console);
}
