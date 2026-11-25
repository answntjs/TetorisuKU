#include "_BlockFactory.h"
#include <algorithm>

BlockFactory::BlockFactory(unsigned int seed)
	: rng_(seed)
{
	refillBag();
}

void BlockFactory::setSeed(unsigned int seed)
{
	rng_.seed(seed);
	bag_.clear();
	refillBag();
}

void BlockFactory::setStickRate(int rate)
{
	stickRate_ = rate;
}

void BlockFactory::refillBag()
{
	bag_ = {
		BlockType::I, BlockType::O, BlockType::T,
		BlockType::J, BlockType::L, BlockType::S, BlockType::Z
	};
	std::shuffle(bag_.begin(), bag_.end(), rng_);
}

std::unique_ptr<Block> BlockFactory::createNextBlock()
{
	BlockType chosen = BlockType::I;

	if (stickRate_ >= 0)
	{
		std::uniform_int_distribution<int> pct(0, 99);
		if (pct(rng_) <= stickRate_)
		{
			chosen = BlockType::I;
		}
		else
		{
			std::uniform_int_distribution<int> others(1, static_cast<int>(BlockType::Count) - 1);
			chosen = static_cast<BlockType>(others(rng_));
		}
	}
	else
	{
		if (bag_.empty())
		{
			refillBag();
		}
		chosen = bag_.back();
		bag_.pop_back();
	}

	auto block = std::make_unique<Block>(chosen, Block::getDefaultColor(chosen));
	block->block_start();
	return block;
}
