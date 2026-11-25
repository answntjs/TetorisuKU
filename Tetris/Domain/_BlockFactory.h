#pragma once

#include "_Block.h"
#include <memory>
#include <random>
#include <vector>

class BlockFactory
{
public:
	explicit BlockFactory(unsigned int seed = std::random_device{}());
	virtual ~BlockFactory() = default;

	void setSeed(unsigned int seed);
	void setStickRate(int rate); // -1: 7-Bag 사용, 0~99: 오리지널 막대 확률 모드

	virtual std::unique_ptr<Block> createNextBlock();
	std::mt19937& getRng() { return rng_; }

private:
	void refillBag();

	std::mt19937 rng_;
	std::vector<BlockType> bag_;
	int stickRate_ = -1;
};
