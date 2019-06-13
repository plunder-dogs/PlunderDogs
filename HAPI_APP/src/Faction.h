#pragma once

#include "Ship.h"
#include "TileArea.h"

constexpr size_t MAX_SHIPS_ALLOWED = 6;

struct Faction
{
	Faction(FactionName factionName, ePlayerType playerType);

	const Ship& getShip(int shipID) const;

	void render(sf::RenderWindow& window, const Map& map, BattlePhase currentBattlePhase);
	bool isEliminated() const;

	//Deployment Phase
	void addShip(FactionName factionName, eShipType shipType);
	bool isAllShipsDeployed() const;
	void createSpawnArea(Map& map);
	bool deployShipAtPosition(Map& map, sf::Vector2i startingPosition, eDirection startingDirection);
	void setShipDeploymentAtPosition(sf::Vector2i startingPosition);
	//Movement Phase
	bool moveShipToPosition(Map& map, int shipID, sf::Vector2i destination);
	bool moveShipToPosition(Map& map, int shipID, sf::Vector2i destination, eDirection endDirection);
	void generateShipMovementGraph(const Map& map, int shipID, sf::Vector2i destination);
	void disableShipMovementGraph(int shipID);
	//Attack Phase
	void shipTakeDamage(int shipID, int damage);
	
	std::vector<Ship> m_ships;
	const FactionName m_factionName;
	const ePlayerType m_playerType;
	TileArea m_spawnArea;
};