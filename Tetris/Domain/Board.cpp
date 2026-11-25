#include "_Board.h"
#include <algorithm>
#include <array>
#include <numeric>

namespace
{
	// SRS Wall Kick offsets: [transitionIndex][testIndex] -> (dx, dy) (dy: +down, -up in our coordinate system)
	// Transition indices:
	// 0: 0->1, 1: 1->2, 2: 2->3, 3: 3->0 (CW)
	// 4: 1->0, 5: 2->1, 6: 3->2, 7: 0->3 (CCW)
	constexpr std::pair<int, int> kJlstzKicks[8][5] = {
		{ {0,0}, {-1,0}, {-1,-1}, {0, 2}, {-1, 2} }, // 0->1
		{ {0,0}, { 1,0}, { 1, 1}, {0,-2}, { 1,-2} }, // 1->2
		{ {0,0}, { 1,0}, { 1,-1}, {0, 2}, { 1, 2} }, // 2->3
		{ {0,0}, {-1,0}, {-1, 1}, {0,-2}, {-1,-2} }, // 3->0
		{ {0,0}, { 1,0}, { 1, 1}, {0,-2}, { 1,-2} }, // 1->0
		{ {0,0}, {-1,0}, {-1,-1}, {0, 2}, {-1, 2} }, // 2->1
		{ {0,0}, {-1,0}, {-1, 1}, {0,-2}, {-1,-2} }, // 3->2
		{ {0,0}, { 1,0}, { 1,-1}, {0, 2}, { 1, 2} }  // 0->3
	};

	constexpr std::pair<int, int> kIKicks[8][5] = {
		{ {0,0}, {-2,0}, { 1,0}, {-2, 1}, { 1,-2} }, // 0->1
		{ {0,0}, {-1,0}, { 2,0}, {-1,-2}, { 2, 1} }, // 1->2
		{ {0,0}, { 2,0}, {-1,0}, { 2,-1}, {-1, 2} }, // 2->3
		{ {0,0}, { 1,0}, {-2,0}, { 1, 2}, {-2,-1} }, // 3->0
		{ {0,0}, { 2,0}, {-1,0}, { 2,-1}, {-1, 2} }, // 1->0
		{ {0,0}, { 1,0}, {-2,0}, { 1, 2}, {-2,-1} }, // 2->1
		{ {0,0}, {-2,0}, { 1,0}, {-2, 1}, { 1,-2} }, // 3->2
		{ {0,0}, {-1,0}, { 2,0}, {-1,-2}, { 2, 1} }  // 0->3
	};

	int getKickTransitionIndex(int fromAngle, bool clockwise)
	{
		if (clockwise)
		{
			return fromAngle; // 0->1:0, 1->2:1, 2->3:2, 3->0:3
		}
		switch (fromAngle)
		{
		case 1: return 4;
		case 2: return 5;
		case 3: return 6;
		case 0: return 7;
		default: return 4;
		}
	}
}

Board::Board(int width, int visibleHeight, int hiddenRows)
	: width_(width)
	, visibleHeight_(visibleHeight)
	, hiddenRows_(hiddenRows)
{
	reset();
}

void Board::reset()
{
	grid_.assign(getTotalHeight(), std::vector<Cell>(width_));
}

int Board::wrapX(int x) const
{
	return ((x % width_) + width_) % width_;
}

bool Board::isCellBlocked(int row, int col) const
{
	if (row >= getTotalHeight())
	{
		return true;
	}
	if (row < 0)
	{
		if (!borderless_ && (col < 0 || col >= width_))
		{
			return true;
		}
		return false;
	}

	if (!borderless_)
	{
		if (col < 0 || col >= width_)
		{
			return true;
		}
		return grid_[row][col].occupied;
	}

	const int wx = wrapX(col);
	return grid_[row][wx].occupied;
}

bool Board::canPlace(const Block& block, int dx, int dy, int testAngle) const
{
	const auto cells = block.getWorldCells(dx, dy, testAngle);
	for (const auto& [cx, cy] : cells)
	{
		if (isCellBlocked(cy, cx))
		{
			return false;
		}
	}
	return true;
}

bool Board::tryRotate(Block& block, bool clockwise) const
{
	if (block.getShape() == BlockType::O)
	{
		block.rotate_block(clockwise);
		return true;
	}

	const int oldAngle = block.getAngle();
	const int newAngle = clockwise ? ((oldAngle + 1) % 4) : ((oldAngle + 3) % 4);
	const int transIdx = getKickTransitionIndex(oldAngle, clockwise);
	const auto* table = (block.getShape() == BlockType::I) ? kIKicks[transIdx] : kJlstzKicks[transIdx];

	for (int i = 0; i < 5; ++i)
	{
		const int dx = table[i].first;
		const int dy = table[i].second;
		if (canPlace(block, dx, dy, newAngle))
		{
			block.moveBy(dx, dy);
			if (borderless_)
			{
				block.setPos(wrapX(block.getX()), block.getY());
			}
			block.setAngle(newAngle);
			block.setLastActionRotate(true);
			return true;
		}
	}
	return false;
}

bool Board::checkTSpin(const Block& block) const
{
	if (block.getShape() != BlockType::T || !block.wasLastActionRotate())
	{
		return false;
	}

	const auto [px, py] = block.getPivotWorld();
	constexpr std::pair<int, int> corners[4] = {
		{-1, -1}, {1, -1}, {-1, 1}, {1, 1}
	};

	int blockedCount = 0;
	for (const auto& [dx, dy] : corners)
	{
		if (isCellBlocked(py + dy, px + dx))
		{
			++blockedCount;
		}
	}
	return blockedCount >= 3;
}

void Board::merge(const Block& block)
{
	const auto cells = block.getWorldCells();
	const Color color = block.getColor();
	const auto effect = block.getDestroyEffect();
	const Color finalColor = (effect != nullptr) ? effect->getTintColor() : color;

	for (const auto& [cx, cy] : cells)
	{
		if (cy < 0 || cy >= getTotalHeight())
		{
			continue;
		}
		int col = cx;
		if (borderless_)
		{
			col = wrapX(col);
		}
		else if (col < 0 || col >= width_)
		{
			continue;
		}

		Cell& target = grid_[cy][col];
		target.occupied = true;
		target.color = finalColor;
		target.clearable = true;
		target.effect = effect;
	}
}

LineClearResult Board::clearFullLines(bool isTSpin)
{
	LineClearResult result;
	result.isTSpin = isTSpin;

	const int totalH = getTotalHeight();
	std::vector<bool> fullRow(totalH, false);

	for (int r = 0; r < totalH; ++r)
	{
		bool full = true;
		for (int c = 0; c < width_; ++c)
		{
			if (!grid_[r][c].occupied || !grid_[r][c].clearable)
			{
				full = false;
				break;
			}
		}

		if (full)
		{
			fullRow[r] = true;
			++result.clearedLines;
			for (int c = 0; c < width_; ++c)
			{
				if (grid_[r][c].effect)
				{
					result.triggeredEffects.push_back(grid_[r][c].effect);
				}
			}
		}
	}

	if (result.clearedLines > 0)
	{
		std::vector<std::vector<Cell>> newGrid(result.clearedLines, std::vector<Cell>(width_));
		newGrid.reserve(totalH);
		for (int r = 0; r < totalH; ++r)
		{
			if (!fullRow[r])
			{
				newGrid.push_back(std::move(grid_[r]));
			}
		}
		grid_ = std::move(newGrid);
	}

	return result;
}

bool Board::pushGarbageLines(int lineCount, int emptyCount, std::mt19937& rng)
{
	if (lineCount <= 0)
	{
		return true;
	}

	const int totalH = getTotalHeight();
	const int clampedEmpty = std::clamp(emptyCount, 1, width_ - 1);

	bool topOverflow = false;
	const int checkRows = std::min(lineCount, totalH);
	for (int r = 0; r < checkRows; ++r)
	{
		for (int c = 0; c < width_; ++c)
		{
			if (grid_[r][c].occupied)
			{
				topOverflow = true;
				break;
			}
		}
		if (topOverflow) break;
	}

	if (lineCount >= totalH)
	{
		grid_.assign(totalH, std::vector<Cell>(width_));
	}
	else
	{
		for (int r = 0; r < totalH - lineCount; ++r)
		{
			grid_[r] = std::move(grid_[r + lineCount]);
		}
	}

	std::vector<int> cols(width_);
	std::iota(cols.begin(), cols.end(), 0);

	const int startRow = std::max(0, totalH - lineCount);
	for (int r = startRow; r < totalH; ++r)
	{
		grid_[r].assign(width_, Cell{ true, Color::DarkGray, true, nullptr });
		std::shuffle(cols.begin(), cols.end(), rng);
		for (int k = 0; k < clampedEmpty; ++k)
		{
			grid_[r][cols[k]] = Cell{};
		}
	}

	return !topOverflow && !isTopOut();
}

int Board::getGhostY(const Block& block) const
{
	int dy = 0;
	while (canPlace(block, 0, dy + 1))
	{
		++dy;
	}
	return block.getY() + dy;
}

bool Board::isTopOut() const
{
	for (int r = 0; r < hiddenRows_; ++r)
	{
		for (int c = 0; c < width_; ++c)
		{
			if (grid_[r][c].occupied)
			{
				return true;
			}
		}
	}
	return false;
}

const Cell& Board::getCell(int row, int col) const
{
	static const Cell kEmpty{};
	if (row < 0 || row >= getTotalHeight() || col < 0 || col >= width_)
	{
		return kEmpty;
	}
	return grid_[row][col];
}

void Board::setCell(int row, int col, const Cell& cell)
{
	if (row < 0 || row >= getTotalHeight() || col < 0 || col >= width_)
	{
		return;
	}
	grid_[row][col] = cell;
}
