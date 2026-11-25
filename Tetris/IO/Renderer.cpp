#include "_Renderer.h"
#include "_TextBox.h"
#include <algorithm>

void Renderer::renderGame(Console& console, const GameManager& gm) const
{
	const Stage* stage = gm.getStage();

	if (stage)
	{
		std::string header = std::string(kGameTitle) + "  |  " + stage->getConfig().name;
		if (gm.isBorderless())
		{
			header += "  |  [BORDERLESS]";
		}
		if (stage->getConfig().garbageIntervalSec > 0.0f)
		{
			const int remain = std::max(0, static_cast<int>(
				stage->getConfig().garbageIntervalSec - stage->getGarbageTimer() + 0.99f));
			header += "  |  GARBAGE IN: " + std::to_string(remain) + "s";
		}
		console.drawText(4, 1, header, Color::Yellow);
		console.drawText(98, 1, "[ESC] PAUSE", Color::DarkGray);
	}

	for (size_t i = 0; i < gm.getPlayerCount(); ++i)
	{
		const Player* p = gm.getPlayer(i);
		if (p)
		{
			renderPlayer(console, *p, stage);
		}
	}

	if (gm.getPlayerCount() == 1)
	{
		TextBox guide(78, 3, 32,
			"LEFT/RIGHT : Move\n"
			"UP / Z     : Rotate CW / CCW\n"
			"DOWN       : Soft Drop\n"
			"SPACE      : Hard Drop\n"
			"LSHIFT     : Hold\n"
			"ESC        : Pause",
			BorderStyle::Single);
		guide.setTitle("CONTROLS");
		guide.setColors(Color::Gray, Color::DarkSkyBlue, Color::Black, true);
		guide.draw(console);
	}
}

void Renderer::renderPlayer(Console& console, const Player& player, const Stage* stage) const
{
	const Board& board = player.getBoard();
	const int ox = player.getViewport().originX;
	const int oy = player.getViewport().originY;
	const int bw = board.getWidth();
	const int vh = board.getVisibleHeight();
	const int hidden = board.getHiddenRows();

	const int stageNum = stage ? stage->getConfig().stageNumber : 1;
	const Color wallColor = static_cast<Color>((stageNum % 6) + 1);

	console.drawText(ox + 1, oy - 1, player.getName(), Color::White);

	const char wallChar = board.isBorderless() ? ':' : '|';
	const Color sideColor = board.isBorderless() ? Color::DarkGray : wallColor;
	for (int r = 0; r < vh; ++r)
	{
		console.drawChar(ox, oy + r, wallChar, sideColor);
		console.drawChar(ox + 1 + bw * 2, oy + r, wallChar, sideColor);
	}

	console.drawChar(ox, oy + vh, '+', wallColor);
	for (int c = 0; c < bw * 2; ++c)
	{
		console.drawChar(ox + 1 + c, oy + vh, '-', wallColor);
	}
	console.drawChar(ox + 1 + bw * 2, oy + vh, '+', wallColor);

	for (int r = 0; r < vh; ++r)
	{
		const int gridRow = r + hidden;
		for (int c = 0; c < bw; ++c)
		{
			const Cell& cell = board.getCell(gridRow, c);
			if (cell.occupied)
			{
				console.drawText(ox + 1 + c * 2, oy + r, "[]", cell.color);
			}
		}
	}

	if (player.isAlive() && player.getCurrentBlock() != nullptr)
	{
		const Block& cur = *player.getCurrentBlock();
		const int ghostY = board.getGhostY(cur);
		const int ghostDy = ghostY - cur.getY();

		if (ghostDy > 0)
		{
			const auto ghostCells = cur.getWorldCells(0, ghostDy);
			for (const auto& [gx, gy] : ghostCells)
			{
				const int vr = gy - hidden;
				if (vr < 0 || vr >= vh) continue;
				const int col = board.isBorderless() ? board.wrapX(gx) : gx;
				if (col < 0 || col >= bw) continue;
				console.drawText(ox + 1 + col * 2, oy + vr, "..", Color::DarkGray);
			}
		}

		const auto activeCells = cur.getWorldCells();
		for (const auto& [ax, ay] : activeCells)
		{
			const int vr = ay - hidden;
			if (vr < 0 || vr >= vh) continue;
			const int col = board.isBorderless() ? board.wrapX(ax) : ax;
			if (col < 0 || col >= bw) continue;
			console.drawText(ox + 1 + col * 2, oy + vr, "[]", cur.getColor());
		}
	}

	const int holdX = ox - 12;
	drawSideBox(console, holdX, oy, 8, 4, "HOLD", player.canHold() ? Color::SkyBlue : Color::DarkGray);
	drawMiniBlock(console, holdX + 1, oy + 1, player.getHoldBlock());

	const int sideX = ox + 2 + bw * 2 + 2;
	drawSideBox(console, sideX, oy, 8, 12, "NEXT", wallColor);
	int nextIdx = 0;
	for (const auto& nb : player.getNextBlocks())
	{
		if (nextIdx >= 3) break;
		drawMiniBlock(console, sideX + 1, oy + 1 + nextIdx * 4, nb.get());
		++nextIdx;
	}

	const int statY = oy + 15;
	console.drawText(sideX, statY, "SCORE", Color::Gray);
	console.drawText(sideX, statY + 1, std::to_string(player.getScore()), Color::White);

	console.drawText(sideX, statY + 3, "LINES", Color::Gray);
	if (stage && stage->getConfig().clearLineGoal > 0 && stage->getConfig().clearLineGoal < 9000)
	{
		const int remain = std::max(0, stage->getConfig().clearLineGoal - player.getStageClearedLines());
		console.drawText(sideX, statY + 4, std::to_string(remain) + " LEFT", Color::White);
	}
	else
	{
		console.drawText(sideX, statY + 4, std::to_string(player.getTotalClearedLines()), Color::White);
	}

	console.drawText(sideX, statY + 6, "COMBO", Color::Gray);
	console.drawText(sideX, statY + 7,
		std::to_string(player.getComboCount()),
		player.getComboCount() > 0 ? Color::Yellow : Color::DarkGray);

	if (player.getLastClearedLines() > 0)
	{
		console.drawText(sideX, statY + 9,
			"CLEARED +" + std::to_string(player.getLastClearedLines()) + "L",
			Color::SkyBlue);
	}

	if (player.getBannerTimer() > 0.0f && !player.getBannerText().empty())
	{
		console.drawText(ox + 1, oy + vh + 1, player.getBannerText(), Color::Yellow);
	}

	if (!player.isAlive())
	{
		console.drawText(ox + 6, oy + 9, "GAME OVER", Color::Red, Color::Black);
	}
}

void Renderer::drawMiniBlock(Console& console, int x, int y, const Block* block) const
{
	if (!block)
	{
		return;
	}
	const auto cells = block->getLocalCells(0);
	for (const auto& [cx, cy] : cells)
	{
		console.drawText(x + cx * 2, y + cy, "[]", block->getColor());
	}
}

void Renderer::drawSideBox(Console& console, int x, int y, int innerCols, int innerRows,
	const std::string& title, Color borderColor) const
{
	console.drawChar(x, y, '+', borderColor);
	console.drawChar(x, y + innerRows + 1, '+', borderColor);
	for (int cx = 1; cx <= innerCols; ++cx)
	{
		console.drawChar(x + cx, y, '-', borderColor);
		console.drawChar(x + cx, y + innerRows + 1, '-', borderColor);
	}
	console.drawChar(x + 1 + innerCols, y, '+', borderColor);
	console.drawChar(x + 1 + innerCols, y + innerRows + 1, '+', borderColor);

	for (int r = 1; r <= innerRows; ++r)
	{
		console.drawChar(x, y + r, '|', borderColor);
		console.drawChar(x + 1 + innerCols, y + r, '|', borderColor);
	}

	if (!title.empty())
	{
		console.drawText(x + 2, y, title, Color::White);
	}
}
