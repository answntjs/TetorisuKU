#include "_BotPlayer.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

BotPlayer::BotPlayer(const std::string& name, const Viewport& viewport,
	bool borderless, unsigned int seed, int difficulty)
	: Player(name, KeyBinding{}, viewport, borderless, seed)
	, difficulty_(std::clamp(difficulty, 1, 5))
{
	static constexpr std::array<float, 5> kActionIntervals = {
		0.42f, 0.30f, 0.21f, 0.14f, 0.085f
	};
	baseActionInterval_ = kActionIntervals[difficulty_ - 1];
}

void BotPlayer::reset(bool borderless, unsigned int seed)
{
	Player::reset(borderless, seed);
	hasPlan_ = false;
	actionTimer_ = 0.0f;
	elapsedTime_ = 0.0f;
}

float BotPlayer::getCurrentActionInterval() const
{
	if (difficulty_ < 5)
	{
		return baseActionInterval_;
	}
	return std::max(0.025f, baseActionInterval_ - elapsedTime_ * 0.0006f);
}

void BotPlayer::update(float dt, const InputHandler&, float dropInterval,
	LineClearResult& outResult, bool& outLocked)
{
	outResult = LineClearResult{};
	outLocked = false;
	updateBanner(dt);

	if (!alive_ || !currentBlock_)
	{
		return;
	}

	elapsedTime_ += dt;

	if (!hasPlan_)
	{
		planBestMove();
		hasPlan_ = true;
	}

	const float actionInterval = getCurrentActionInterval();
	actionTimer_ += dt;
	if (actionTimer_ >= actionInterval)
	{
		actionTimer_ -= actionInterval;

		if (currentBlock_->getAngle() != targetAngle_)
		{
			if (!rotate(true))
			{
				// Fallback if blocked: drop where we are
				hardDrop(outResult, outLocked);
				hasPlan_ = false;
				return;
			}
		}
		else if (currentBlock_->getX() != targetX_)
		{
			const int w = board_.getWidth();
			int dx = targetX_ - currentBlock_->getX();
			if (board_.isBorderless())
			{
				if (dx > w / 2) dx -= w;
				if (dx < -w / 2) dx += w;
			}

			bool moved = (dx < 0) ? moveLeft() : moveRight();
			if (!moved)
			{
				hardDrop(outResult, outLocked);
				hasPlan_ = false;
				return;
			}
		}
		else
		{
			hardDrop(outResult, outLocked);
			hasPlan_ = false;
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
			hasPlan_ = false;
		}
	}
}

void BotPlayer::planBestMove()
{
	double bestScore = -std::numeric_limits<double>::infinity();
	targetAngle_ = currentBlock_->getAngle();
	targetX_ = currentBlock_->getX();

	const int w = board_.getWidth();
	const int minX = board_.isBorderless() ? 0 : -2;
	const int maxX = board_.isBorderless() ? (w - 1) : (w + 1);

	for (int angle = 0; angle < 4; ++angle)
	{
		for (int x = minX; x <= maxX; ++x)
		{
			Block simBlock = *currentBlock_;
			simBlock.setAngle(angle);
			simBlock.setPos(x, currentBlock_->getY());

			if (!board_.canPlace(simBlock))
			{
				continue;
			}

			while (board_.canPlace(simBlock, 0, 1))
			{
				simBlock.moveBy(0, 1);
			}

			Board simBoard = board_;
			simBoard.merge(simBlock);
			const LineClearResult clr = simBoard.clearFullLines(false);
			const double score = evaluateBoard(simBoard, clr.clearedLines);

			if (score > bestScore)
			{
				bestScore = score;
				targetAngle_ = angle;
				targetX_ = board_.isBorderless() ? board_.wrapX(x) : x;
			}
		}
	}
}

double BotPlayer::evaluateBoard(const Board& simBoard, int clearedLines) const
{
	const int w = simBoard.getWidth();
	const int totalH = simBoard.getTotalHeight();

	std::vector<int> heights(w, 0);
	int holes = 0;

	for (int c = 0; c < w; ++c)
	{
		bool foundTop = false;
		for (int r = 0; r < totalH; ++r)
		{
			if (simBoard.getCell(r, c).occupied)
			{
				if (!foundTop)
				{
					heights[c] = totalH - r;
					foundTop = true;
				}
			}
			else if (foundTop)
			{
				++holes;
			}
		}
	}

	int aggregateHeight = 0;
	int bumpiness = 0;
	for (int c = 0; c < w; ++c)
	{
		aggregateHeight += heights[c];
		if (c + 1 < w)
		{
			bumpiness += std::abs(heights[c] - heights[c + 1]);
		}
	}

	return -0.51 * aggregateHeight + 0.76 * clearedLines - 0.36 * holes - 0.18 * bumpiness;
}
