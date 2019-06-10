#pragma once

#include <memory>
#include "Timer.h"
#include "Global.h"
#include "SpriteToggleVisibility.h"
#include <SFML/Graphics.hpp>
#include <queue>
#include "Sprite.h"
#include <array>

constexpr size_t MOVEMENT_GRAPH_SIZE{ 32 };

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
	sf::Vector2i getCurrentPosition() const;
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
	sf::Vector2i getEndOfMovementPath() const;

	void update(float deltaTime, const Map& map);
	void render(sf::RenderWindow& window, const Map& map);
	void setDestination();
	void onNewTurn();
	void enableAction();
	void disableAction();

	//Deployment Phase
	void setDeploymentPosition(sf::Vector2i position);
	void deployAtPosition(sf::Vector2i position, eDirection startingDirection = eDirection::eNorth);
	int generateMovementPath(const Map& map, sf::Vector2i destination);
	void disableMovementPath();
	//Movement Phase
	bool move(Map& map, sf::Vector2i destination);
	bool move(Map& map, sf::Vector2i destination, eDirection endDirection);
	//Attack Phase
	void takeDamage(int damageAmount);
	void fireWeapon();

private:
	const FactionName m_factionName;
	const eShipType m_shipType;
	const int m_ID;
	sf::Vector2i m_currentPosition;
	std::queue<posi> m_movementPath;
	Timer m_movementTimer;
	int m_movementPathSize;
	eDirection m_currentDirection;
	bool m_weaponFired;
	bool m_isDead;
	Sprite m_actionSprite;
	bool m_movingToDestination;
	bool m_destinationSet;
	int m_maxHealth;
	int m_health;
	int m_damage;
	int m_range;
	int m_movementPoints;
	Sprite m_sprite;
	bool m_deployed;
	std::array<Sprite, MOVEMENT_GRAPH_SIZE> m_movementGraph;

	unsigned int getDirectionCost(int currentDirection, int newDirection);
	void disableMovementPathNode(sf::Vector2i position, const Map& map);
	void handleRotation();
};