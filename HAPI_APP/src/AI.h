#pragma once

#include <memory>
#include "Global.h"

class Battle;
struct Tile;
class Map;
struct BattleEntity;
struct BattlePlayer;
struct Player;
struct EntityProperties;

namespace AI
{
	void handleMovementPhase(const Battle& battle, Map& map, BattlePlayer& player, int currentUnit);
	void handleShootingPhase(Battle& battle, const Map& map, BattlePlayer& player, int currentUnit);
	void handleDeploymentPhase(Battle& battle, Map& map, BattlePlayer& bPlayer, const Player& player);
	void handleShipSelection(std::vector<EntityProperties>& shipPool, std::vector<EntityProperties*>& selectedShips);
};