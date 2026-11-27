#include "_Player.h"
#include <algorithm>

Player::Player(const std::string& name, const KeyBinding& keyBinding, const Viewport& viewport,
	bool borderless, unsigned int seed)
	: name_(name)
	, factory_(seed)
	, keyBinding_(keyBinding)
	, viewport_(viewport)
{
	reset(borderless, seed);
}

void Player::reset(bool borderless, unsigned int seed)
{
	board_.reset();
	board_.setBorderless(borderless);
	factory_.setSeed(seed);
	garbageRng_.seed(seed ^ 0x9e3779b9u);
	holdBlock_.reset();
	nextBlocks_.clear();
	canHold_ = true;
	comboCount_ = 0;
	lastClearedLines_ = 0;
	backToBack_ = false;
	score_ = 0;
	totalClearedLines_ = 0;
	stageClearedLines_ = 0;
	alive_ = true;
	dropTimer_ = 0.0f;
	bannerText_.clear();
	bannerTimer_ = 0.0f;
	items_.clear();

	for (int i = 0; i < NextPreviewCount; ++i)
	{
		nextBlocks_.push_back(factory_.createNextBlock());
	}
	spawnNextBlock();
}

void Player::spawnNextBlock()
{
	fakeNextPreview_.reset();
	if (nextBlocks_.empty())
	{
		nextBlocks_.push_back(factory_.createNextBlock());
	}
	currentBlock_ = std::move(nextBlocks_.front());
	nextBlocks_.pop_front();
	nextBlocks_.push_back(factory_.createNextBlock());

	currentBlock_->block_start(3, 2);
	if (!board_.canPlace(*currentBlock_))
	{
		currentBlock_->block_start(3, 0);
		if (!board_.canPlace(*currentBlock_))
		{
			alive_ = false;
		}
	}
}

void Player::update(float dt, const InputHandler& input, float dropInterval,
	LineClearResult& outResult, bool& outLocked)
{
	outResult = LineClearResult{};
	outLocked = false;
	updateBanner(dt);

	if (!alive_ || !currentBlock_)
	{
		return;
	}

	if (input.isKeyPressed(keyBinding_.hold))
	{
		hold();
		if (!alive_)
		{
			return;
		}
	}
	if (input.isKeyPressed(keyBinding_.rotateCW))
	{
		rotate(true);
	}
	if (input.isKeyPressed(keyBinding_.rotateCCW))
	{
		rotate(false);
	}
	if (input.isKeyPressed(keyBinding_.moveLeft, true))
	{
		moveLeft();
	}
	if (input.isKeyPressed(keyBinding_.moveRight, true))
	{
		moveRight();
	}
	if (input.isKeyPressed(keyBinding_.hardDrop))
	{
		hardDrop(outResult, outLocked);
		return;
	}

	if (input.isKeyPressed(keyBinding_.softDrop, true))
	{
		if (softDrop())
		{
			score_ += 1;
			dropTimer_ = 0.0f;
		}
		else
		{
			lockCurrentBlock(outResult, outLocked);
			return;
		}
	}

	dropTimer_ += dt;
	if (dropTimer_ >= dropInterval)
	{
		dropTimer_ -= dropInterval;
		if (!softDrop())
		{
			lockCurrentBlock(outResult, outLocked);
		}
	}
}

bool Player::moveLeft()
{
	if (!currentBlock_ || !board_.canPlace(*currentBlock_, -1, 0))
	{
		return false;
	}
	currentBlock_->moveBy(-1, 0);
	if (board_.isBorderless())
	{
		currentBlock_->setPos(board_.wrapX(currentBlock_->getX()), currentBlock_->getY());
	}
	currentBlock_->setLastActionRotate(false);
	return true;
}

bool Player::moveRight()
{
	if (!currentBlock_ || !board_.canPlace(*currentBlock_, 1, 0))
	{
		return false;
	}
	currentBlock_->moveBy(1, 0);
	if (board_.isBorderless())
	{
		currentBlock_->setPos(board_.wrapX(currentBlock_->getX()), currentBlock_->getY());
	}
	currentBlock_->setLastActionRotate(false);
	return true;
}

bool Player::softDrop()
{
	if (!currentBlock_ || !board_.canPlace(*currentBlock_, 0, 1))
	{
		return false;
	}
	currentBlock_->moveBy(0, 1);
	currentBlock_->setLastActionRotate(false);
	return true;
}

void Player::hardDrop(LineClearResult& outResult, bool& outLocked)
{
	outResult = LineClearResult{};
	outLocked = false;
	if (!currentBlock_)
	{
		return;
	}

	int dropped = 0;
	while (board_.canPlace(*currentBlock_, 0, 1))
	{
		currentBlock_->moveBy(0, 1);
		currentBlock_->setLastActionRotate(false);
		++dropped;
	}
	score_ += dropped * 2;
	lockCurrentBlock(outResult, outLocked);
}

bool Player::rotate(bool clockwise)
{
	if (!currentBlock_)
	{
		return false;
	}
	return board_.tryRotate(*currentBlock_, clockwise);
}

bool Player::hold()
{
	if (!canHold_ || !currentBlock_)
	{
		return false;
	}

	if (!holdBlock_)
	{
		holdBlock_ = std::move(currentBlock_);
		holdBlock_->block_start();
		spawnNextBlock();
	}
	else
	{
		std::swap(currentBlock_, holdBlock_);
		holdBlock_->block_start();
		currentBlock_->block_start(3, 2);
		if (!board_.canPlace(*currentBlock_))
		{
			currentBlock_->block_start(3, 0);
			if (!board_.canPlace(*currentBlock_))
			{
				alive_ = false;
			}
		}
	}

	canHold_ = false;
	dropTimer_ = 0.0f;
	return true;
}

void Player::lockCurrentBlock(LineClearResult& outResult, bool& outLocked)
{
	outLocked = true;
	const bool isTSpin = board_.checkTSpin(*currentBlock_);
	board_.merge(*currentBlock_);

	if (board_.isTopOut())
	{
		alive_ = false;
	}

	outResult = board_.clearFullLines(isTSpin);

	if (outResult.clearedLines > 0)
	{
		comboCount_ += outResult.clearedLines;
		outResult.comboCount = comboCount_;
		lastClearedLines_ = outResult.clearedLines;

		const bool isDifficult = (outResult.clearedLines == 4) || outResult.isTSpin;
		outResult.isBackToBack = isDifficult && backToBack_;
		backToBack_ = isDifficult;

		totalClearedLines_ += outResult.clearedLines;
		stageClearedLines_ += outResult.clearedLines;

		int baseScore = 0;
		if (outResult.isTSpin)
		{
			static constexpr int kTSpinScores[5] = { 400, 800, 1200, 1600, 1600 };
			baseScore = kTSpinScores[std::min(outResult.clearedLines, 4)];
		}
		else
		{
			static constexpr int kLineScores[5] = { 0, 100, 300, 500, 800 };
			baseScore = kLineScores[std::min(outResult.clearedLines, 4)];
		}

		if (outResult.isBackToBack)
		{
			baseScore = baseScore * 3 / 2;
		}
		const int comboBonus = (comboCount_ > 1) ? (comboCount_ - 1) * 50 : 0;
		outResult.lineClearScore = baseScore + comboBonus;
		score_ += outResult.lineClearScore;

		if (outResult.isTSpin)
		{
			setBanner("T-SPIN +" + std::to_string(outResult.clearedLines)
				+ "L (COMBO " + std::to_string(comboCount_) + ")");
		}
		else if (outResult.clearedLines == 4)
		{
			setBanner("TETRIS +4L (COMBO " + std::to_string(comboCount_) + ")");
		}
		else
		{
			setBanner("+" + std::to_string(outResult.clearedLines)
				+ (outResult.clearedLines == 1 ? " LINE" : " LINES")
				+ " (COMBO " + std::to_string(comboCount_) + ")");
		}
	}
	else
	{
		comboCount_ = 0;
		outResult.comboCount = 0;
		lastClearedLines_ = 0;
		if (isTSpin)
		{
			score_ += 400;
			setBanner("T-SPIN!");
		}
	}

	canHold_ = true;
	dropTimer_ = 0.0f;
	if (alive_)
	{
		spawnNextBlock();
	}
}

bool Player::receiveGarbage(int nLines, int xHoles)
{
	if (!alive_ || nLines <= 0)
	{
		return alive_;
	}

	return finishRowRise(board_.pushGarbageLines(nLines, xHoles, garbageRng_), nLines);
}

bool Player::receivePermanentLine()
{
	return alive_ && finishRowRise(board_.pushPermanentLine(), 1);
}

bool Player::finishRowRise(bool boardSurvived, int lines)
{
	if (!boardSurvived)
	{
		alive_ = false;
		return false;
	}

	if (currentBlock_)
	{
		int safety = 0;
		while (!board_.canPlace(*currentBlock_) && safety < lines + 4)
		{
			currentBlock_->moveBy(0, -1);
			++safety;
		}
		if (!board_.canPlace(*currentBlock_))
		{
			alive_ = false;
			return false;
		}
	}
	return true;
}

bool Player::stealHoldBlock()
{
	if (!holdBlock_)
	{
		return false;
	}
	holdBlock_.reset();
	return true;
}

bool Player::fakeNextPreview(std::mt19937& rng)
{
	if (fakeNextPreview_ || nextBlocks_.empty())
	{
		return false;
	}
	std::uniform_int_distribution<int> choose(1, static_cast<int>(BlockType::Count) - 1);
	const int realShape = static_cast<int>(nextBlocks_.front()->getShape());
	fakeNextPreview_ = static_cast<BlockType>((realShape + choose(rng)) % static_cast<int>(BlockType::Count));
	return true;
}

bool Player::lockCurrentRotation()
{
	if (!currentBlock_ || currentBlock_->isRotationLocked())
	{
		return false;
	}
	currentBlock_->lockRotation();
	return true;
}

bool Player::markHostage(const std::shared_ptr<IBlockEffect>& marker, std::mt19937& rng)
{
	return board_.markHostageCell(marker, rng);
}

void Player::removeHostageMarker(const IBlockEffect& marker)
{
	board_.removeCellEffect(marker);
}

void Player::addItem(std::unique_ptr<IItem> item)
{
	if (item)
	{
		items_.push_back(std::move(item));
	}
}

bool Player::useFirstItem(GameManager& gm)
{
	if (items_.empty())
	{
		return false;
	}
	auto item = std::move(items_.front());
	items_.erase(items_.begin());
	item->use(*this, gm);
	return true;
}

void Player::setBanner(const std::string& text, float duration)
{
	bannerText_ = text;
	bannerTimer_ = duration;
}

void Player::updateBanner(float dt)
{
	if (bannerTimer_ > 0.0f)
	{
		bannerTimer_ -= dt;
		if (bannerTimer_ <= 0.0f)
		{
			bannerTimer_ = 0.0f;
			bannerText_.clear();
		}
	}
}
