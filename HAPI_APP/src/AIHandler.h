#pragma once

#include "TileArea.h"

class Battle;
class Map;
struct Faction;
class AIHandler
{
public:
	static AIHandler& getInstance()
	{
		static AIHandler instance;
		return instance;
	}

	void handleMovementPhase(const Battle& battle, Map& map, Faction& player, int currentUnit);
	void handleShootingPhase(Battle& battle, const Map& map, Faction& player, int currentUnit);
	void handleDeploymentPhase(Battle& battle, const Faction& currentPlayer);
	void loadShips(Faction& player);

private:
	std::vector<Ray2D> m_movementArea;
	std::vector<const Tile*> m_targetArea;
};