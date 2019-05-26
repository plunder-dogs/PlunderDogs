#pragma once

#include <memory>

class Battle;
class Map;
struct Player;
namespace AI
{
	void handleMovementPhase(const Battle& battle, Map& map, std::unique_ptr<Player>& player, int currentUnit);
	void handleShootingPhase(Battle& battle, const Map& map, std::unique_ptr<Player>& player, int currentUnit);
	void handleDeploymentPhase(Battle& battle, Map& map, const Player& currentPlayer);
	void handleShipSelection(Player& player);
};