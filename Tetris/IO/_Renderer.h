#pragma once

#include "_Console.h"
#include "../Core/_GameManager.h"
#include "../Domain/_Player.h"

class Renderer
{
public:
	void renderGame(Console& console, const GameManager& gm) const;
	void renderPlayer(Console& console, const Player& player, const Stage* stage) const;

private:
	void drawMiniBlock(Console& console, int x, int y, const Block* block) const;
	void drawSideBox(Console& console, int x, int y, int innerCols, int innerRows,
		const std::string& title, Color borderColor) const;
};
