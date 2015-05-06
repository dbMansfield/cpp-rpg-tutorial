#ifndef BATTLE_HPP
#define BATTLE_HPP

#include "creature.hpp"
#include "player.hpp"
#include "dialogue.hpp"

#include <algorithm>
#include <iostream>
#include <vector>
#include <queue>
#include <map>

enum class BattleEventType { ATTACK, DEFEND };

class BattleEvent
{
	public:

	Creature* source;
	Creature* target;
	BattleEventType type;

	BattleEvent(Creature* source, Creature* target, BattleEventType type)
	{
		this->source = source;
		this->target = target;
		this->type = type;
	}

	int run()
	{
		switch(type)
		{
			case BattleEventType::ATTACK:
				return source->attack(target);
			case BattleEventType::DEFEND:
				return 0;
			default:
				return 0;
		}
	}
};

class Battle
{
	private:

	// All the creatures that are participating in the fight.
	// We assume the player is a Creature with id "player".
	// A vector is used because we need to get the nth element
	// for use with a Dialogue
	std::vector<Creature*> combatants;

	Dialogue battleOptions;

	void kill(Creature* creature)
	{
		// Find the creature's position in the combatants vector
		auto pos = std::find(this->combatants.begin(), this->combatants.end(), creature);

		// Don't try and delete the creature if it doesn't exist
		if(pos != this->combatants.end())
		{
			std::cout << creature->name << " is slain!\n";

			// Remove the creature
			this->combatants.erase(pos);
		}

		return;
	}

	public:

	Battle(std::vector<Creature*>& combatants)
	{
		this->combatants = combatants;

		// Construct the menu
		this->battleOptions = Dialogue("What will you do?",
		{
			"Attack",
			"Defend"
		});

		// Store the unique creature names and whether there is
		// only one or more of them
		std::map<std::string, int> names;
		for(auto com : this->combatants)
		{
			if(names.count(com->name) < 2 && com->id != "player")
			{
				names[com->name] = names.count(com->name);
			}
		}

		// Create a unique name based on the number of times
		// that name is present in the battle
		for(auto& com : this->combatants)
		{
			std::string newName = com->name;
			if(names.count(com->name) > 0)
			{
				// Append (1) to the end of the name, and then increase the
				// number for the next creature.
				newName += " (" + std::to_string(names[com->name]) + ")";
				names[com->name] += 1;
			}
			// Change the creature name to the new one
			com->name = newName;
		}
	}

	void run()
	{
		while(this->combatants.size() > 1)
		{
			this->next_turn();
		}

		return;
	}

	void next_turn()
	{
		// Queue of battle events. Fastest combatants will be
		// at the start of the queue, and so will go first,
		// whereas slower ones will be at the back
		std::queue<BattleEvent> events;

		// Sort the combatants in agility order
		std::sort(combatants.begin(), combatants.end(), [](Creature* a, Creature* b) { return a->agility < b->agility; });

		// Iterate over the combatants and decide what they should do,
		// before adding the action to the event queue.
		for(auto com : this->combatants)
		{
			if(com->id == "player")
			{
				// Create the target selection dialogue
				Dialogue targetSelection = Dialogue("Who?", {});
				// Created every turn because some combatants may die
				for(auto target : this->combatants)
				{
					if(target->id != "player")
					{
						targetSelection.addChoice(target->name);
					}
				}

				// Ask the player for their action (attack or defend)
				int choice = this->battleOptions.activate();

				switch(choice)
				{
					default:
					case 1:
					{
						// Player is attacking, so ask for the target.
						// Dialogue returns the number of the choice, but we
						// want the pointer that name corresponds to and so we
						// must look it up in the combatant vector
						Creature* target = this->combatants[targetSelection.activate()-1];
						// Add the attack command to the event queue
						events.push(BattleEvent(com, target, BattleEventType::ATTACK));
						break;
					}
					case 2:
					{
						// Player is defending, so do nothing
						events.push(BattleEvent(com, nullptr, BattleEventType::DEFEND));
						break;
					}
				}
			}
			else
			{
				// Simple enemy AI where enemy constantly attacks player
				Creature* player = *std::find_if(this->combatants.begin(), this->combatants.end(),
					[](Creature* a) { return a->id == "player"; });

				events.push(BattleEvent(com, player, BattleEventType::ATTACK));
			}
		}

		// Take each event from the queue in turn and process them,
		// displaying the results
		while(!events.empty())
		{
			// Take event from the front of the queue
			BattleEvent event = events.front();
			switch(event.type)
			{
				case BattleEventType::ATTACK:
				{
					// The event can't be run if either the source or the
					// target were slain previously in this turn, so we
					// must check that they're valid first
					auto a = this->combatants.begin();
					auto b = this->combatants.end();
					if(std::find(a, b, event.source) == b || std::find(a, b, event.target) == b)
					{
						break;
					}
					std::cout << event.source->name
						<< " attacks "
						<< event.target->name
						<< " for "
						<< event.run()
						<< " damage!\n";
					// Delete slain enemies
					if(event.target->hp <= 0)
					{
						this->kill(event.target);
					}
					break;
				}
				case BattleEventType::DEFEND:
					std::cout << event.source->name
						<< " defends!\n";
					break;
				default:
					break;
			}
			events.pop();
		}
	}
};

#endif /* BATTLE_HPP */