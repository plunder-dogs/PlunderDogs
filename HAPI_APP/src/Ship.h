#pragma once

#include <memory>
#include "Timer.h"
#include "Global.h"
#include <queue>
#include "Sprite.h"
#include <array>
#include "TileArea.h"

constexpr size_t MOVEMENT_GRAPH_SIZE{ 32 };

struct GameEvent;
struct Tile;
struct Weapons;
class Map;
class Ship
{
public:
	Ship(FactionName playerName, eShipType shipType, int ID);
	Ship(Ship& orig);
	~Ship();

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

	void update(float deltaTime);
	void render(sf::RenderWindow& window, const Map& map);
	void renderMovementGraph(sf::RenderWindow& window, const Map& map);
	void setDestination();
	void enableAction();
	void disableAction();

	//Deployment Phase
	void setDeploymentPosition(sf::Vector2i position);
	void deployAtPosition(sf::Vector2i position, eDirection startingDirection = eDirection::eNorth);
	void generateMovementArea(const Map& map, sf::Vector2i destination);
	void disableMovementGraph();
	//Movement Phase
	void startMovement(Map& map);
	void startMovement(Map& map, eDirection endDirection);
	//Attack Phase
	void takeDamage(int damageAmount);
	void fireWeapon();

private:
	const FactionName m_factionName;
	const eShipType m_shipType;
	const int m_ID;
	sf::Vector2i m_currentPosition;
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
	PosiArea m_movementArea;

	unsigned int getDirectionCost(int currentDirection, int newDirection);
	void disableMovementGraphNode(sf::Vector2i position);
	void onNewBattlePhase(GameEvent gameEvent);
};