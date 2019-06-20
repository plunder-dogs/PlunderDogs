#pragma once

#include "Ship.h"
#include "TileArea.h"

struct Faction
{
	Faction(FactionName factionName, ePlayerType playerType);

	const Ship& getShip(int shipID) const;
	void render(sf::RenderWindow& window, const Map& map, BattlePhase currentBattlePhase);
	void renderShipsMovementGraphs(sf::RenderWindow& window, const Map& map);
	bool isEliminated() const;
	void clearSpawnArea();

	//Deployment Phase
	bool isPositionInDeploymentArea(sf::Vector2i position) const;
	void addShip(FactionName factionName, eShipType shipType);
	bool isAllShipsDeployed() const;
	void createSpawnArea(Map& map);
	void deployShipAtPosition(Map& map, sf::Vector2i startingPosition, eDirection startingDirection);
	void setShipDeploymentAtPosition(sf::Vector2i startingPosition);
	//Movement Phase
	void moveShipToPosition(Map& map, int shipID);
	void moveShipToPosition(Map& map, int shipID, eDirection endDirection);
	void generateShipMovementGraph(const Map& map, int shipID, sf::Vector2i destination);
	void disableShipMovementGraph(int shipID);
	//Attack Phase
	void shipTakeDamage(int shipID, int damage);
	
	std::vector<Ship> m_ships;
	const FactionName m_factionName;
	const ePlayerType m_playerType;
	TileArea m_spawnArea;
};