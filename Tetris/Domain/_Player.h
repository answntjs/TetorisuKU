#pragma once

#include "_Block.h"
#include "_BlockFactory.h"
#include "_Board.h"
#include "../Core/_IItem.h"
#include "../IO/_InputHandler.h"
#include "../Core/_Types.h"
#include <deque>
#include <memory>
#include <string>
#include <vector>

class Player
{
public:
	static constexpr int NextPreviewCount = 3;

	Player(const std::string& name, const KeyBinding& keyBinding, const Viewport& viewport,
		bool borderless = false, unsigned int seed = std::random_device{}());
	virtual ~Player() = default;

	virtual void reset(bool borderless, unsigned int seed);
	virtual void update(float dt, const InputHandler& input, float dropInterval,
		LineClearResult& outResult, bool& outLocked);

	bool moveLeft();
	bool moveRight();
	bool softDrop();
	void hardDrop(LineClearResult& outResult, bool& outLocked);
	bool rotate(bool clockwise = true);
	bool hold();

	bool receiveGarbage(int nLines, int xHoles);
	void addItem(std::unique_ptr<IItem> item);
	bool useFirstItem(GameManager& gm);

	void setBorderless(bool borderless) { board_.setBorderless(borderless); }
	void setStickRate(int rate) { factory_.setStickRate(rate); }
	void resetStageClearedLines() { stageClearedLines_ = 0; }
	void setBanner(const std::string& text, float duration = 1.5f);

	const std::string& getName() const { return name_; }
	const Board& getBoard() const { return board_; }
	const Block* getCurrentBlock() const { return currentBlock_.get(); }
	const Block* getHoldBlock() const { return holdBlock_.get(); }
	const std::deque<std::unique_ptr<Block>>& getNextBlocks() const { return nextBlocks_; }
	const KeyBinding& getKeyBinding() const { return keyBinding_; }
	const Viewport& getViewport() const { return viewport_; }
	void setViewport(const Viewport& vp) { viewport_ = vp; }

	int getScore() const { return score_; }
	int getComboCount() const { return comboCount_; }
	int getLastClearedLines() const { return lastClearedLines_; }
	int getTotalClearedLines() const { return totalClearedLines_; }
	int getStageClearedLines() const { return stageClearedLines_; }
	bool isAlive() const { return alive_; }
	bool canHold() const { return canHold_; }
	const std::string& getBannerText() const { return bannerText_; }
	float getBannerTimer() const { return bannerTimer_; }
	const std::vector<std::unique_ptr<IItem>>& getItems() const { return items_; }

protected:
	void spawnNextBlock();
	void lockCurrentBlock(LineClearResult& outResult, bool& outLocked);
	void updateBanner(float dt);

	std::string name_;
	Board board_;
	BlockFactory factory_;
	std::unique_ptr<Block> currentBlock_;
	std::unique_ptr<Block> holdBlock_;
	std::deque<std::unique_ptr<Block>> nextBlocks_;
	bool canHold_ = true;

	KeyBinding keyBinding_;
	Viewport viewport_;

	int comboCount_ = 0;
	int lastClearedLines_ = 0;
	bool backToBack_ = false;
	int score_ = 0;
	int totalClearedLines_ = 0;
	int stageClearedLines_ = 0;
	bool alive_ = true;
	float dropTimer_ = 0.0f;

	std::string bannerText_;
	float bannerTimer_ = 0.0f;

	std::vector<std::unique_ptr<IItem>> items_;
};
