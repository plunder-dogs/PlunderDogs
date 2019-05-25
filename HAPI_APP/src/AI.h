#pragma once

#include <memory>
#include "Global.h"

class Battle;
struct Tile;
class Map;
class Ship;
struct Player;
struct Player;
struct ShipGlobalProperties;

namespace AI
{
	void handleMovementPhase(const Battle& battle, Map& map, std::unique_ptr<Player>& player, int currentUnit);
	void handleShootingPhase(Battle& battle, const Map& map, std::unique_ptr<Player>& player, int currentUnit);
	//void handleDeploymentPhase(Battle& battle, Map& map, Player& bPlayer, const Player& player);
	//void handleShipSelection(std::vector<ShipGlobalProperties>& shipPool, std::vector<ShipGlobalProperties*>& selectedShips);
};