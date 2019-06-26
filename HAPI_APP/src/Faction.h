#pragma once

#include "Ship.h"

struct Faction
{
	Faction();

	bool isActive() const;
	bool isPositionInSpawnArea(sf::Vector2i position) const;
	const std::vector<Ship>& getAllShips() const;
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
	void setShipDeploymentAtPosition(sf::Vector2i startingPosition, eDirection direction);
	//Movement Phase
	void moveShipToPosition(Map& map, int shipID);
	void moveShipToPosition(Map& map, int shipID, eDirection endDirection);
	void generateShipMovementArea(const Map& map, int shipID, sf::Vector2i destination, bool displayOnlyLastPosition = false);
	void clearShipMovementArea(int shipID);
	//Attack Phase
	void shipTakeDamage(int shipID, int damage);
	
	std::vector<Ship> m_ships;
	FactionName m_factionName;
	ePlayerType m_playerType;
	TileArea m_spawnArea;
};