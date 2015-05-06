#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <cmath>
#include "JsonBox.h"

#include "player.hpp"
#include "creature.hpp"

Player::Player(std::string name, int hp, int strength, int agility, double evasion,
	unsigned int xp, unsigned int level, std::string className) :
	Creature("player", name, hp, strength, agility, evasion, xp)
{
	this->level = level;
	this->className = className;
}

Player::Player() : Player::Player("", 0, 0, 0, 0.0, 0, 1, "nullid")
{
}

Player::Player(JsonBox::Value v, EntityManager* mgr) : Player::Player()
{
	this->load("player", v, mgr);
}

// Calculates the total experience required to reach a certain level
unsigned int Player::xpToLevel(unsigned int level)
{
	return (unsigned int)(1.5 * std::pow(this->level, 3));
}

// Level the player to the next level if it has enough experience
// to do so, returning true if it could level up and false otherwise.
bool Player::levelUp()
{
	// Can't level up if there's not enough experience
	if(this->xp < xpToLevel(this->level+1))
	{
		return false;
	}

	// Advance to the next level
	++level;

	// Variables to keep track of stat changes, and their associated
	// multipliers, which depend on the class. The multiplier affects
	// how much that stat increases each level, and is higher if the
	// class specialises in that stat
	// [hp, strength, agility]
	unsigned int statIncreases[3] = {0, 0, 0};
	float statMultipliers[3] = {0, 0, 0};
	statMultipliers[0] = 1.3;
	statMultipliers[1] = this->className == "Fighter" ? 8.0 : 6.0;
	statMultipliers[2] = this->className == "Rogue" ? 8.0 : 6.0;

	// Compute the stat increases for each stat
	for(int i = 0; i < 3; ++i)
	{
		float base = std::tanh(this->level / 30.0) * ((this->level % 2) + 1);
		statIncreases[i] += int(1 + statMultipliers[i] * base);
	}

	// Adjust all of the stats accordingly
	this->hp += statIncreases[0];
	this->maxHp += statIncreases[0];
	this->strength += statIncreases[1];
	this->agility += statIncreases[2];

	// Tell the user that they grew a level, what the increases were
	// and what their stats are now
	std::cout << this->name << " grew to level " << level << "!\n";
	std::cout << "Health   +" << statIncreases[0] << " -> " << this->maxHp << std::endl;
	std::cout << "Strength +" << statIncreases[1] << " -> " << this->strength << std::endl;
	std::cout << "Agility  +" << statIncreases[2] << " -> " << this->agility << std::endl;
	std::cout << "----------------\n";

	return true;
}

JsonBox::Object Player::toJson()
{
	JsonBox::Object o = Creature::toJson();

	o["className"] = JsonBox::Value(this->className);
	o["level"] = JsonBox::Value(int(this->level));

	return o;
}

void Player::save()
{
	JsonBox::Value v(this->toJson());
	v.writeToFile(this->name + ".json");

}

// Attempt to load all data from the JSON value
void Player::load(std::string id, JsonBox::Value v, EntityManager* mgr)
{
	// Load data shared with Creature
	Creature::load(id, v, mgr);

	// Load optional variables
	JsonBox::Object o = v.getObject();

	this->className = o["className"].getString();
	this->level = o["level"].getInteger();

	return;
}
