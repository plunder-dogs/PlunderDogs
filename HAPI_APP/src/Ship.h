#pragma once

#include <memory>
#include "Timer.h"
#include "Global.h"
#include "SpriteToggleVisibility.h"
#include <SFML/Graphics.hpp>

//TODO: Temp

struct Tile;
struct Weapons;
class Map;
class Ship
{
public:
	Ship(FactionName playerName, eShipType shipType, int ID);
	Ship(Ship& orig);

	FactionName getFactionName() const;
	eDirection getCurrentDirection() const;
	eShipType getShipType() const;
	std::pair<int, int> getCurrentPosition() const;
	bool isWeaponFired() const;
	bool isDead() const;
	bool isMovingToDestination() const;
	bool isDestinationSet() const;
	bool isDeployed() const;
	int getMovementPoints() const;
	int getRange() const;
	int getDamage() const;
	int getHealth() const;
	int getID() const;
	std::pair<int, int> getEndOfMovementPath() const;

	void update(float deltaTime, const Map& map);
	void render(const Map& map) const;
	void setDestination();
	void onNewTurn();
	void enableAction();
	void disableAction();

	//Deployment Phase
	void setDeploymentPosition(std::pair<int, int> position);
	void deployAtPosition(std::pair<int, int> position, eDirection startingDirection = eDirection::eNorth);
	int generateMovementPath(const Map& map, std::pair<int, int> destination);
	void disableMovementPath();
	//Movement Phase
	bool move(Map& map, std::pair<int, int> destination);
	bool move(Map& map, std::pair<int, int> destination, eDirection endDirection);
	//Attack Phase
	void takeDamage(int damageAmount);
	void fireWeapon();

private:
	const FactionName m_factionName;
	const eShipType m_shipType;
	const int m_ID;
	std::pair<int, int> m_currentPosition;
	std::queue<posi> m_pathToTile;
	Timer m_movementTimer;
	int m_movementPathSize;
	eDirection m_currentDirection;
	bool m_weaponFired;
	bool m_isDead;
	SpriteToggleVisibility m_actionSprite;
	bool m_movingToDestination;
	bool m_destinationSet;
	int m_maxHealth;
	int m_health;
	int m_damage;
	int m_range;
	int m_movementPoints;
	std::unique_ptr<Sprite> m_sprite;
	bool m_deployed;
	std::vector<SpriteToggleVisibility> m_movementPath;

	unsigned int getDirectionCost(int currentDirection, int newDirection);
	void disableMovementPathNode(std::pair<int, int> position, const Map& map);
	void handleRotation();
};