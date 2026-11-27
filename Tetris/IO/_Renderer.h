#pragma once

#include "_Console.h"
#include "../Core/_GameManager.h"
#include "../Domain/_Player.h"

class Renderer
{
public:
	void renderGame(Console& console, const GameManager& gm) const;
	void renderPlayer(Console& console, const Player& player, const Stage* stage) const;
	void renderStoryBriefing(Console& console, const GameManager& gm, bool handoff) const;
	void renderStoryPromotion(Console& console, const std::string& result) const;
	void drawStoryPortrait(Console& console, int x, int y, int portrait) const;

private:
	void drawPursuit(Console& console, const PursuitStatus& pursuit, const GameManager& gm) const;
	void drawMiniBlock(Console& console, int x, int y, const Block* block) const;
	void drawSideBox(Console& console, int x, int y, int innerCols, int innerRows,
		const std::string& title, Color borderColor) const;
};
