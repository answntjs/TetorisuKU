#include "_GameOverScene.h"
#include "../Core/_Game.h"
#include "../Domain/_PursuitStage.h"
#include "_PlayScene.h"
#include "../IO/_TextBox.h"
#include "_TitleScene.h"

GameOverScene::GameOverScene(Game& game, const GameManager& match)
	: Scene(game)
	, message_(match.getEndMessage())
	, mode_(match.getMode())
	, restartLevel_(match.getStartLevel())
	, borderless_(match.isBorderless())
	, storyCase_(match.getStage()->getConfig().stageNumber)
	, victory_(match.isVictory())
{
	for (size_t i = 0; i < stats_.size(); ++i)
	{
		if (const auto* player = match.getPlayer(i))
		{
			stats_[i] = player->getName() + "   SCORE " + std::to_string(player->getScore())
				+ "   LINES " + std::to_string(player->getTotalClearedLines());
		}
	}
	if (const auto* pursuit = match.getStage()->getPursuitStatus())
	{
		pursuitSummary_ = "추격 점수  " + std::to_string(pursuit->score) + " / "
			+ std::to_string(pursuit->target) + "\n";
	}
	if (mode_ == GameMode::StorySolo)
	{
		restartLevel_ = victory_ ? 1 : storyCase_;
	}
}

void GameOverScene::handleInput(const InputHandler& input)
{
	if (input.isKeyPressed(VK_RETURN) || input.isKeyPressed(VK_SPACE) || input.isKeyPressed('T'))
	{
		game_.changeScene(std::make_unique<TitleScene>(game_));
	}
	else if (input.isKeyPressed('R'))
	{
		game_.changeScene(std::make_unique<PlayScene>(game_, mode_, restartLevel_, borderless_));
	}
}

void GameOverScene::update(float)
{
}

void GameOverScene::render(Console& console, const Renderer& renderer) const
{
	if (showsPromotion())
	{
		renderer.renderStoryPromotion(console, message_);
		return;
	}
	std::string body = message_ + "\n\n" + pursuitSummary_;
	for (const auto& stats : stats_)
	{
		if (!stats.empty())
		{
			body += stats + "\n";
		}
	}
	body += "\n[R] 다시 하기    [ENTER / SPACE] 타이틀";

	if (isStoryMode(mode_))
	{
		console.fillRect(14, 10, 92, 17, ' ', Color::White, Color::Black);
		renderer.drawStoryPortrait(console, 17, 12, 0);
		renderer.drawStoryPortrait(console, 91, 12, mode_ == GameMode::StorySolo ? storyCase_ : StoryCaseCount);
	}
	TextBox box(31, 10, 56, body, BorderStyle::Bold);
	box.setTitle(isStoryMode(mode_) ? "추격 결과" : "RESULT");
	box.setColors(Color::White, victory_ ? Color::SkyBlue : Color::Red, Color::Black, true);
	box.draw(console);
}
