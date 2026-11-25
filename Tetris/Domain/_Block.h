#pragma once

#include "../Core/_IBlockEffect.h"
#include "../Core/_Types.h"
#include <array>
#include <memory>

class Block
{
public:
	Block(BlockType shape = BlockType::I, Color color = Color::SkyBlue,
		std::shared_ptr<IBlockEffect> destroyEffect = nullptr);
	virtual ~Block() = default;

	void block_start(int spawnX = 3, int spawnY = 2);
	void rotate_block(bool clockwise = true);

	std::array<std::pair<int, int>, 4> getLocalCells(int testAngle = -1) const;
	std::array<std::pair<int, int>, 4> getWorldCells(int dx = 0, int dy = 0, int testAngle = -1) const;
	std::pair<int, int> getPivotWorld() const;

	BlockType getShape() const { return shape_; }
	int getAngle() const { return angle_; }
	const std::pair<int, int>& getPos() const { return pos_; }
	int getX() const { return pos_.first; }
	int getY() const { return pos_.second; }
	Color getColor() const { return color_; }
	bool wasLastActionRotate() const { return lastActionWasRotate_; }
	std::shared_ptr<IBlockEffect> getDestroyEffect() const { return destroyEffect_; }

	void setPos(int x, int y) { pos_ = { x, y }; }
	void moveBy(int dx, int dy) { pos_.first += dx; pos_.second += dy; }
	void setAngle(int angle) { angle_ = ((angle % 4) + 4) % 4; }
	void setLastActionRotate(bool rotated) { lastActionWasRotate_ = rotated; }
	void setDestroyEffect(std::shared_ptr<IBlockEffect> effect) { destroyEffect_ = std::move(effect); }

	static Color getDefaultColor(BlockType shape);

private:
	BlockType shape_;
	int angle_ = 0;
	std::pair<int, int> pos_{ 3, 2 };
	Color color_;
	bool lastActionWasRotate_ = false;
	std::shared_ptr<IBlockEffect> destroyEffect_;
};
