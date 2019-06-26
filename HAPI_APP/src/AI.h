#pragma once

#include <memory>

class Battle;
class Map;
struct Faction;
namespace AI
{
	void handleMovementPhase(const Battle& battle, Map& map, Faction& player, int currentUnit);
	void handleShootingPhase(Battle& battle, const Map& map, Faction& player, int currentUnit);
	void handleDeploymentPhase(Battle& battle, const Faction& currentPlayer);
	void loadInPlayerShips(Faction& player);
}; 