#pragma once

#include "Ship.h"

struct SpawnNode
{
	SpawnNode(FactionName factionName, std::pair<int, int> position);

	void render(const Map& map) const;

	std::pair<int, int> m_position;
	std::unique_ptr<Sprite> m_sprite;
};

struct Faction
{
	Faction(FactionName name, ePlayerType playerType);

	const Ship& getShip(int shipID) const;

	void render(const Map& map) const;
	void onNewTurn();

	//Deployment Phase
	void addShip(FactionName factionName, eShipType shipType);
	bool isAllShipsDeployed() const;
	void createSpawnArea(Map& map);
	bool deployShipAtPosition(Map& map, std::pair<int, int> startingPosition, eDirection startingDirection);
	bool setShipDeploymentAtPosition(std::pair<int, int> startingPosition);
	//Movement Phase
	bool moveShipToPosition(Map& map, int shipID, std::pair<int, int> destination);
	bool moveShipToPosition(Map& map, int shipID, std::pair<int, int> destination, eDirection endDirection);
	void generateShipMovementPath(const Map& map, int shipID, std::pair<int, int> destination);
	void disableShipMovementPath(int shipID);
	//Attack Phase
	void shipTakeDamage(int shipID, int damage);

	std::vector<Ship> m_ships;
	const FactionName m_factionName;
	const ePlayerType m_playerType;
	bool m_eliminated;
	Ship* m_shipToDeploy;
	std::vector<SpawnNode> m_spawnArea;
};