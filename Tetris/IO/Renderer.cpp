#include "_Renderer.h"
#include "_TextBox.h"
#include "../Domain/_PursuitStage.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <string_view>

namespace
{
	using Portrait = std::array<std::string_view, 12>;
	constexpr std::array<Portrait, StoryCaseCount + 1> kPortraits{{
		// Police
		{{ "............", "....BBBB....", "...BBYYBB...", "..BBBBBBBB..",
		   "..KKKKKKKK..", "...SKSSKS...", "...SSSSSS...", "....SSSS....",
		   "...BBWWBB...", "..BBBYBBBB..", "..BBBBBBBB..", "..KKKKKKKK.." }},
		// Red Cap
		{{ "............", "....RRR.....", "...RRRRRR...", "..RRRRRRRR..",
		   "..RRKKKK....", "...SKSSKS...", "...SSSSSS...", "....SSSS....",
		   "...RRKKRR...", "..RRRRRRRR..", "..RKKRRKKR..", "..KKKKKKKK.." }},
		// Silver Hand
		{{ "............", "....GGGG....", "...GGGGGG...", "..GGGKKGGG..",
		   "..GGSSSSGG..", "...SKSSKS...", "...KKKKKK...", "....KKKK....",
		   "...GGKKGG...", "..GGGGGGGG..", "..GGGGGWWG..", "..KKKKKKKK.." }},
		// False Gentleman
		{{ "....KKKK....", "....KVVK....", "....KKKK....", "..KKKKKKKK..",
		   "...SSSSSS...", "...KWKSKW...", "...SSWSSS...", "....SSSS....",
		   "...VVWVVV...", "..VVWYWVVV..", "..VVVYVVVV..", "..KKKKKKKK.." }},
		// Golden Mask
		{{ "............", "....KKKK....", "...KKKKKK...", "..KKYYYYKK..",
		   "..KYYKKYYK..", "..KYWYYWYK..", "...YYYYYY...", "....YYYY....",
		   "...GGKKGG...", "..GGYYYYGG..", "..GGGGGGGG..", "..KKKKKKKK.." }},
		// Black Shadow
		{{ "............", "....VVVV....", "...VVKKVV...", "..VVKKKKVV..",
		   "..VKKKKKKV..", "..VKWKKWKV..", "..VKKKKKKV..", "...VVKKVV...",
		   "...KVVVVK...", "..KKKVVKKK..", "..KKKKKKKK..", "..VVVVVVVV.." }}
	}};

	Color portraitColor(char pixel)
	{
		switch (pixel)
		{
		case 'B': return Color::Blue;
		case 'Y': return Color::Yellow;
		case 'S': return Color::DarkYellow;
		case 'W': return Color::White;
		case 'R': return Color::Red;
		case 'G': return Color::Gray;
		case 'K': return Color::DarkGray;
		case 'V': return Color::Violet;
		default: return Color::Black;
		}
	}
}

void Renderer::drawStoryPortrait(Console& console, int x, int y, int portrait) const
{
	const auto& pixels = kPortraits[std::clamp(portrait, 0, StoryCaseCount)];
	for (size_t row = 0; row < pixels.size(); row += 2)
	{
		for (size_t col = 0; col < pixels[row].size(); ++col)
		{
			console.drawGlyph(x + static_cast<int>(col), y + static_cast<int>(row / 2),
				"▀", 1, portraitColor(pixels[row][col]), portraitColor(pixels[row + 1][col]));
		}
	}
}

void Renderer::renderStoryBriefing(Console& console, const GameManager& gm, bool promotion) const
{
	const bool solo = gm.getMode() == GameMode::StorySolo;
	const int caseNumber = gm.getStage()->getConfig().stageNumber;
	drawStoryPortrait(console, 16, 13, 0);
	drawStoryPortrait(console, 92, 13, solo ? caseNumber : StoryCaseCount);
	console.drawText(17, 20, solo ? "경찰" : "1P 경찰", Color::SkyBlue);
	console.drawText(93, 20, solo ? "용의자" : "2P 도둑", Color::Violet);

	std::string body;
	if (solo)
	{
		const auto& suspect = getStoryCase(caseNumber);
		if (promotion)
		{
			body += "사건을 해결했다. 경찰이 진급해 새 사건을 맡는다.\n\n";
		}
		body += gm.getStage()->getConfig().name + "\n";
		body += std::string(suspect.suspect) + "  \"" + suspect.dialogue + "\"\n\n";
		body += "줄을 지우면 용의자와의 거리가 줄어든다.\n";
		body += "체포 목표  " + std::to_string(gm.getStage()->getPursuitStatus()->target) + "점\n";
	}
	else
	{
		body = "경찰  \"이번엔 놓치지 않는다.\"\n"
			"도둑  \"따라올 수 있으면 와 봐.\"\n\n"
			"경찰은 3000점에 체포, 도둑은 0점에 도주.\n"
			"1P WASD / SPACE / LSHIFT\n"
			"2P 방향키 / RSHIFT / ENTER\n";
	}
	body += "\n[ENTER / SPACE] 추격 시작";

	TextBox box(31, 9, 56, body, BorderStyle::Bold);
	box.setTitle(promotion ? "진급" : solo ? "수사 브리핑" : "경찰과 도둑");
	box.setColors(Color::White, Color::SkyBlue, Color::Black, true);
	box.draw(console);
}

void Renderer::drawPursuit(Console& console, const PursuitStatus& pursuit, bool duel) const
{
	constexpr int trackStart = 39;
	constexpr int trackLength = 41;
	const int gap = pursuit.target - pursuit.score;
	const int marker = trackStart + 1 + gap * (trackLength - 3) / pursuit.target;
	console.drawText(4, 3, "경찰과 도둑의 거리", Color::SkyBlue);
	console.fillRect(trackStart, 4, trackLength, 1, '.', Color::DarkGray);
	console.drawChar(trackStart, 4, '[', Color::SkyBlue);
	console.drawChar(trackStart + trackLength - 1, 4, ']', Color::Violet);
	console.drawChar(trackStart + 1, 3, 'P', Color::SkyBlue);
	console.drawChar(marker, 4, 'T', Color::Violet);
	console.drawText(84, 3, "추격 " + std::to_string(pursuit.score) + " / "
		+ std::to_string(pursuit.target), Color::Yellow);
	console.drawText(84, 4, "남은 거리 " + std::to_string(gap), Color::Gray);

	if (duel)
	{
		if (pursuit.attackTime[0] > 0.0f)
		{
			const int travel = static_cast<int>((0.8f - pursuit.attackTime[0]) * 26.0f);
			console.drawText(trackStart + travel, 5, (travel % 4 < 2) ? "*|==>" : "+|==>", Color::SkyBlue);
			console.drawText(4, 5, "경찰 차단선", Color::SkyBlue);
		}
		if (pursuit.attackTime[1] > 0.0f)
		{
			const int travel = static_cast<int>((0.8f - pursuit.attackTime[1]) * 26.0f);
			console.drawText(trackStart + trackLength - 5 - travel, 6, "<-[]", Color::Violet);
			console.drawText(84, 6, "도둑의 투척", Color::Violet);
		}
	}
	else if (pursuit.hostageTime > 0.0f)
	{
		console.drawText(4, 6, "인질 구출  " + std::to_string(static_cast<int>(std::ceil(pursuit.hostageTime)))
			+ "초  |  H!가 있는 줄을 지우세요", Color::Yellow);
	}
	if (pursuit.noticeTime > 0.0f)
	{
		console.drawText(4, 7, std::string(pursuit.notice), Color::Yellow);
	}
}

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
		if (const auto* pursuit = stage->getPursuitStatus())
		{
			drawPursuit(console, *pursuit, gm.getMode() == GameMode::StoryDuel);
		}
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
		TextBox guide(78, isStoryMode(gm.getMode()) ? 9 : 3, 32,
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
				const char* glyph = cell.effect ? cell.effect->getCellGlyph() : cell.clearable ? "[]" : "##";
				const Color tint = cell.effect ? cell.effect->getTintColor() : cell.color;
				console.drawText(ox + 1 + c * 2, oy + r, glyph, tint);
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
			console.drawText(ox + 1 + col * 2, oy + vr,
				cur.isRotationLocked() ? "!!" : "[]",
				cur.isRotationLocked() ? Color::Yellow : cur.getColor());
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
		if (const auto fake = player.getFakeNextPreview(); nextIdx == 0 && fake)
		{
			const Block decoy(*fake, Block::getDefaultColor(*fake));
			drawMiniBlock(console, sideX + 1, oy + 1, &decoy);
		}
		else
		{
			drawMiniBlock(console, sideX + 1, oy + 1 + nextIdx * 4, nb.get());
		}
		++nextIdx;
	}

	const int statY = oy + 15;
	console.drawText(sideX, statY, "SCORE", Color::Gray);
	console.drawText(sideX, statY + 1, std::to_string(player.getScore()), Color::White);

	console.drawText(sideX, statY + 3, "LINES", Color::Gray);
	if (stage && stage->getConfig().clearLineGoal > 0)
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
	else if (player.getCurrentBlock() && player.getCurrentBlock()->isRotationLocked())
	{
		console.drawText(ox + 1, oy + vh + 2, "!! 회전 불가", Color::Yellow);
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
