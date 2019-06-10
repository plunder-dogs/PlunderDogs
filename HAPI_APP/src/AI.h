#pragma once

#include <memory>

class Battle;
class Map;
struct Faction;
namespace AI
{
	void handleMovementPhase(const Battle& battle, Map& map, std::unique_ptr<Faction>& player, int currentUnit);
	void handleShootingPhase(Battle& battle, const Map& map, std::unique_ptr<Faction>& player, int currentUnit);
	void handleDeploymentPhase(Battle& battle, Map& map, const Faction& currentPlayer);
	void loadInPlayerShips(Faction& player);
}; 