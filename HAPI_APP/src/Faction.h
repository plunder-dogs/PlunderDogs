#pragma once

#include "Ship.h"
#include "Utilities/NonCopyable.h"

struct Faction : private NonCopyable
{
	Faction(eFactionName factionName);

	bool isActive() const;
	bool isPositionInSpawnArea(sf::Vector2i position) const;
	const std::vector<Ship>& getAllShips() const;
	const Ship& getShip(int shipID) const;
	void render(sf::RenderWindow& window, const Map& map, eBattlePhase currentBattlePhase, int currentFactionTurn);
	void renderShipsMovementGraphs(sf::RenderWindow& window, const Map& map);
	bool isEliminated() const;
	void clearSpawnArea();

	//Deployment Phase
	bool isPositionInDeploymentArea(sf::Vector2i position) const;
	void addShip(eFactionName factionName, eShipType shipType);
	bool isAllShipsDeployed() const;
	void createSpawnArea(const Map& map, sf::Vector2i spawnPosition );
	void deployShipAtPosition(Map& map, sf::Vector2i startingPosition, eDirection startingDirection);
	void setShipDeploymentAtPosition(sf::Vector2i startingPosition, eDirection direction);
	//Movement Phase
	void moveShipToPosition(Map& map, int shipID);
	void moveShipToPosition(Map& map, int shipID, eDirection endDirection);
	void generateShipMovementArea(const Map& map, int shipID, sf::Vector2i destination, bool displayOnlyLastPosition = false);
	void rectifyShipMovementArea(int shipID);
	void clearShipMovementArea(int shipID);
	//Attack Phase
	void shipTakeDamage(int shipID, int damage);
	
	std::vector<Ship> m_ships;
	const eFactionName m_factionName;
	eFactionControllerType m_controllerType;
	TileArea m_spawnArea;
};