#pragma once

#include <string>

class Player;
class GameManager;

class IItem
{
public:
	virtual ~IItem() = default;

	virtual void use(Player& user, GameManager& gm) = 0;
	virtual std::string getName() const = 0;
	virtual std::string getDescription() const = 0;
};
