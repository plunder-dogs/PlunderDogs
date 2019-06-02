#pragma once

#include "Ship.h"

struct SpawnNode
{
	SpawnNode(FactionName factionName, sf::Vector2i position);

	void render(sf::RenderWindow& window, const Map& map);

	sf::Vector2i m_position;
	sf::Sprite m_sprite;
};

struct Faction
{
	Faction(FactionName name, ePlayerType playerType);

	const Ship& getShip(int shipID) const;

	void render(sf::RenderWindow& window, const Map& map);
	void onNewTurn();

	//Deployment Phase
	void addShip(FactionName factionName, eShipType shipType);
	bool isAllShipsDeployed() const;
	void createSpawnArea(Map& map);
	bool deployShipAtPosition(Map& map, sf::Vector2i startingPosition, eDirection startingDirection);
	bool setShipDeploymentAtPosition(sf::Vector2i startingPosition);
	//Movement Phase
	bool moveShipToPosition(Map& map, int shipID, sf::Vector2i destination);
	bool moveShipToPosition(Map& map, int shipID, sf::Vector2i destination, eDirection endDirection);
	void generateShipMovementPath(const Map& map, int shipID, sf::Vector2i destination);
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