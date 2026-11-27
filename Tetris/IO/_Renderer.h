#pragma once

#include "_Console.h"
#include "../Core/_GameManager.h"
#include "../Domain/_Player.h"

class Renderer
{
public:
	void renderGame(Console& console, const GameManager& gm) const;
	void renderPlayer(Console& console, const Player& player, const Stage* stage) const;
	void renderStoryBriefing(Console& console, const GameManager& gm, bool promotion) const;
	void drawStoryPortrait(Console& console, int x, int y, int portrait) const;

private:
	void drawPursuit(Console& console, const PursuitStatus& pursuit, bool duel) const;
	void drawMiniBlock(Console& console, int x, int y, const Block* block) const;
	void drawSideBox(Console& console, int x, int y, int innerCols, int innerRows,
		const std::string& title, Color borderColor) const;
};
