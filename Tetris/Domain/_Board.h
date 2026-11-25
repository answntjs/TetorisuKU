#pragma once

#include "_Block.h"
#include "../Core/_IBlockEffect.h"
#include "../Core/_Types.h"
#include <memory>
#include <random>
#include <vector>

struct Cell
{
	bool occupied = false;
	Color color = Color::Black;
	bool clearable = true;
	std::shared_ptr<IBlockEffect> effect = nullptr;
};

class Board
{
public:
	static constexpr int DefaultWidth = 10;
	static constexpr int DefaultVisibleHeight = 20;
	static constexpr int DefaultHiddenRows = 4;

	Board(int width = DefaultWidth, int visibleHeight = DefaultVisibleHeight, int hiddenRows = DefaultHiddenRows);

	void reset();
	void setBorderless(bool borderless) { borderless_ = borderless; }
	bool isBorderless() const { return borderless_; }

	int getWidth() const { return width_; }
	int getVisibleHeight() const { return visibleHeight_; }
	int getHiddenRows() const { return hiddenRows_; }
	int getTotalHeight() const { return visibleHeight_ + hiddenRows_; }

	int wrapX(int x) const;
	bool canPlace(const Block& block, int dx = 0, int dy = 0, int testAngle = -1) const;
	bool tryRotate(Block& block, bool clockwise = true) const;
	bool checkTSpin(const Block& block) const;

	void merge(const Block& block);
	LineClearResult clearFullLines(bool isTSpin);
	bool pushGarbageLines(int lineCount, int emptyCount, std::mt19937& rng);

	int getGhostY(const Block& block) const;
	bool isTopOut() const;

	const Cell& getCell(int row, int col) const;
	void setCell(int row, int col, const Cell& cell);

private:
	bool isCellBlocked(int row, int col) const;

	int width_;
	int visibleHeight_;
	int hiddenRows_;
	bool borderless_ = false;
	std::vector<std::vector<Cell>> grid_;
};
