#pragma once

#include "_Types.h"
#include <string>

class IBlockEffect
{
public:
	virtual ~IBlockEffect() = default;

	virtual void onDestroy(Player& owner, GameManager& gm, int row, int col) = 0;
	virtual std::string getName() const = 0;
	virtual Color getTintColor() const = 0;
	virtual const char* getCellGlyph() const { return "[]"; }
};
