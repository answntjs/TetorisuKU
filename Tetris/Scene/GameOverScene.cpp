#include "_GameOverScene.h"
#include "../Core/_Game.h"
#include "_PlayScene.h"
#include "../IO/_TextBox.h"
#include "_TitleScene.h"

GameOverScene::GameOverScene(Game& game, const std::string& message, int score, int lines,
	GameMode mode, int startLevel, bool borderless)
	: Scene(game)
	, message_(message)
	, score_(score)
	, lines_(lines)
	, mode_(mode)
	, startLevel_(startLevel)
	, borderless_(borderless)
{
}

void GameOverScene::handleInput(const InputHandler& input)
{
	if (input.isKeyPressed(VK_RETURN) || input.isKeyPressed(VK_SPACE) || input.isKeyPressed('T'))
	{
		game_.changeScene(std::make_unique<TitleScene>(game_));
	}
	else if (input.isKeyPressed('R'))
	{
		game_.changeScene(std::make_unique<PlayScene>(game_, mode_, startLevel_, borderless_));
	}
}

void GameOverScene::update(float)
{
}

void GameOverScene::render(Console& console, const Renderer&) const
{
	std::string body = message_ + "\n\n";
	body += "  SCORE : " + std::to_string(score_) + "\n";
	body += "  LINES : " + std::to_string(lines_) + "\n\n";
	body += "  [R]           Restart\n";
	body += "  [ENTER/SPACE] Title Menu";

	TextBox box(40, 11, 34, body, BorderStyle::Bold);
	box.setTitle("RESULT");
	box.setColors(Color::White, Color::Red, Color::Black, true);
	box.draw(console);
}
