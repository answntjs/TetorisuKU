#include "_Renderer.h"
#include "_StoryPresentation.h"
#include "_TextBox.h"
#include "../Domain/_PursuitStage.h"
#include <algorithm>
#include <cmath>

void Renderer::drawStoryPortrait(Console& console, int x, int y, int portrait) const
{
	StoryPresentation::drawPortrait(console, x, y, portrait);
}

void Renderer::renderStoryBriefing(Console& console, const GameManager& gm, bool handoff) const
{
	const bool solo = gm.getMode() == GameMode::StorySolo;
	const int caseNumber = gm.getStage()->getConfig().stageNumber;
	drawStoryPortrait(console, 12, 12, 0);
	drawStoryPortrait(console, 92, 12, solo ? caseNumber : StoryCaseCount);
	console.drawText(17, 21, solo ? "형사" : "1P 경찰", Color::SkyBlue);
	console.drawText(93, 21, solo ? "용의자" : "2P 도둑", Color::Violet);

	std::string body;
	if (solo)
	{
		if (handoff)
		{
			StoryPresentation::drawIcon(console, 17, 8, StoryPresentation::Icon::Arrest);
		}
		body = gm.getStage()->getConfig().name + "\n\n";
		body += StoryPresentation::briefing(caseNumber, handoff);
		body += "\n\n줄을 지우면 용의자와의 거리가 줄어든다.\n";
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
	box.setTitle(handoff ? "다음 사건" : solo ? "수사 브리핑" : "경찰과 도둑");
	box.setColors(Color::White, Color::SkyBlue, Color::Black, true);
	box.draw(console);
}

void Renderer::renderStoryPromotion(Console& console, const std::string& result) const
{
	drawStoryPortrait(console, 51, 3, 0);
	StoryPresentation::drawIcon(console, 56, 12, StoryPresentation::Icon::Badge);

	std::string body = result + "\n\n팀장  \"";
	body += StoryPresentation::Promotion;
	body += "\"\n\n[ENTER / SPACE] 타이틀    [R] 처음부터";
	TextBox box(31, 17, 56, body, BorderStyle::Bold);
	box.setTitle("포상 진급식");
	box.setColors(Color::White, Color::Yellow, Color::Black, true);
	box.draw(console);
}

void Renderer::drawPursuit(Console& console, const PursuitStatus& pursuit, const GameManager& gm) const
{
	using StoryPresentation::Icon;
	const bool duel = gm.getMode() == GameMode::StoryDuel;
	const int caseNumber = gm.getStage()->getConfig().stageNumber;
	const int gap = pursuit.target - pursuit.score;
	constexpr int policeX = 38;
	constexpr int nearestSuspectX = policeX + 7;
	constexpr int farthestSuspectX = 74;
	const int marker = nearestSuspectX + gap * (farthestSuspectX - nearestSuspectX) / pursuit.target;
	console.drawText(4, 2, duel ? "경찰과 도둑의 거리" : "용의자와의 거리", Color::SkyBlue);
	console.fillRect(policeX + 2, 5, 40, 1, '.', Color::DarkGray);
	console.fillRect(policeX + 3, 5, marker - policeX, 1, '-', Color::Violet);
	StoryPresentation::drawIcon(console, policeX, 2, Icon::Police);
	StoryPresentation::drawIcon(console, marker, 2, gap == 0 ? Icon::Arrest : Icon::Suspect,
		duel ? StoryCaseCount : caseNumber);
	console.drawText(84, 2, "추격 " + std::to_string(pursuit.score) + " / "
		+ std::to_string(pursuit.target), Color::Yellow);
	console.drawText(84, 4, gap == 0 ? "체포" : "남은 거리 " + std::to_string(gap), Color::Gray);

	if (duel)
	{
		if (pursuit.attackTime[0] > 0.0f)
		{
			const int travel = static_cast<int>((0.8f - pursuit.attackTime[0]) * 26.0f);
			console.drawText(policeX + travel, 6, (travel % 4 < 2) ? "*|==>" : "+|==>", Color::SkyBlue);
			console.drawText(4, 6, "경찰 차단선", Color::SkyBlue);
		}
		if (pursuit.attackTime[1] > 0.0f)
		{
			const int travel = static_cast<int>((0.8f - pursuit.attackTime[1]) * 26.0f);
			console.drawText(farthestSuspectX - travel, 7, "<-[]", Color::Violet);
			console.drawText(84, 7, "도둑의 투척", Color::Violet);
		}
		return;
	}

	if (const auto* current = gm.getPlayer(0)->getCurrentBlock(); current && current->isRotationLocked())
	{
		StoryPresentation::drawIcon(console, 4, 3, Icon::RotationLock);
		console.drawText(12, 4, "회전 불가", Color::Yellow);
	}
	if (pursuit.hostageTime > 0.0f)
	{
		console.drawText(4, 5, "인질  " + std::to_string(static_cast<int>(std::ceil(pursuit.hostageTime)))
			+ "초  |  H! 줄 삭제", Color::Yellow);
	}
	if (pursuit.notice.timeLeft > 0.0f)
	{
		const auto line = StoryPresentation::dialogue(pursuit.notice.cue, caseNumber);
		if (pursuit.notice.cue == PursuitCue::HostageExpired)
		{
			console.drawText(4, 6, std::string(line), Color::Yellow);
		}
		else
		{
			console.drawText(4, 6, "용의자  \"" + std::string(line) + "\"", Color::Violet);
		}
	}
	if (pursuit.fear.timeLeft > 0.0f)
	{
		console.drawText(4, 7, "용의자  \"" + std::string(StoryPresentation::FearOpening) + "\"", Color::Violet);
		if (pursuit.fear.cue == PursuitCue::FearRise)
		{
			console.drawText(12, 8, std::string(StoryPresentation::dialogue(pursuit.fear.cue, caseNumber)),
				Color::Red);
		}
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
			header += "  |  [좌우 연결]";
		}
		if (stage->getConfig().garbageIntervalSec > 0.0f)
		{
			const int remain = std::max(0, static_cast<int>(
				stage->getConfig().garbageIntervalSec - stage->getGarbageTimer() + 0.99f));
			header += "  |  방해 줄: " + std::to_string(remain) + "초";
		}
		console.drawText(4, 1, header, Color::Yellow);
		console.drawText(96, 1, "[ESC] 일시정지", Color::DarkGray);
		if (const auto* pursuit = stage->getPursuitStatus())
		{
			drawPursuit(console, *pursuit, gm);
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
		TextBox guide(78, gm.getPlayer(0)->getViewport().originY, 34,
			"좌/우  : 이동\n"
			"위 / Z : 시계 / 반시계 회전\n"
			"아래   : 소프트 드롭\n"
			"SPACE  : 하드 드롭\n"
			"LSHIFT : 홀드\n"
			"ESC    : 일시정지",
			BorderStyle::Single);
		guide.setTitle("조작키");
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
	drawSideBox(console, holdX, oy, 8, 4, "홀드", player.canHold() ? Color::SkyBlue : Color::DarkGray);
	drawMiniBlock(console, holdX + 1, oy + 1, player.getHoldBlock());

	const int sideX = ox + 2 + bw * 2 + 2;
	drawSideBox(console, sideX, oy, 8, 12, "다음", wallColor);
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
	console.drawText(sideX, statY, "점수", Color::Gray);
	console.drawText(sideX, statY + 1, std::to_string(player.getScore()), Color::White);

	console.drawText(sideX, statY + 3, "줄", Color::Gray);
	if (stage && stage->getConfig().clearLineGoal > 0)
	{
		const int remain = std::max(0, stage->getConfig().clearLineGoal - player.getStageClearedLines());
		console.drawText(sideX, statY + 4, std::to_string(remain) + "줄 남음", Color::White);
	}
	else
	{
		console.drawText(sideX, statY + 4, std::to_string(player.getTotalClearedLines()), Color::White);
	}

	console.drawText(sideX, statY + 6, "콤보", Color::Gray);
	console.drawText(sideX, statY + 7,
		std::to_string(player.getComboCount()),
		player.getComboCount() > 0 ? Color::Yellow : Color::DarkGray);

	if (player.getLastClearedLines() > 0)
	{
		console.drawText(sideX, statY + 9,
			"삭제 +" + std::to_string(player.getLastClearedLines()) + "줄",
			Color::SkyBlue);
	}

	if (player.getBannerTimer() > 0.0f && !player.getBannerText().empty())
	{
		console.drawText(ox + 1, oy + vh + 1, player.getBannerText(), Color::Yellow);
	}

	if (!player.isAlive())
	{
		console.drawText(ox + 6, oy + 9, "게임 오버", Color::Red, Color::Black);
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
