#pragma once

#include "Timer.h"
#include "Global.h"
#include "Sprite.h"
#include "TileArea.h"

constexpr size_t MOVEMENT_GRAPH_SIZE{ 32 };

struct Faction;
struct GameEvent;
struct Tile;
struct Weapons;
class Map;
class Ship
{
public:
	Ship(FactionName playerName, eShipType shipType, int ID);
	~Ship();

	const std::deque<Ray2D>& getMovementArea() const;
	sf::FloatRect getAABB(const Map& map) const;
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
	void renderMovementArea(sf::RenderWindow& window, const Map& map);
	void setDestination();
	void enableAction();
	void disableAction();

	//Deployment Phase
	void setDeploymentPosition(sf::Vector2i position, eDirection direction);
	void deployAtPosition(sf::Vector2i position, eDirection startingDirection = eDirection::eNorth);
	//Movement Phase
	void startMovement(Map& map);
	void startMovement(Map& map, eDirection endDirection);
	void generateMovementArea(const Faction& faction, const Map& map, sf::Vector2i destination, bool displayOnlyLastPosition = false);
	//Disallow ending position of ships movement area to overlap with 
	//ship belonging to same faction
	void rectifyMovementArea(const Faction& faction);
	void clearMovementArea();
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
	bool m_displayOnlyLastPosition;
	int m_maxHealth;
	int m_health;
	int m_damage;
	int m_range;
	int m_movementPoints;
	Sprite m_sprite;
	bool m_deployed;
	PosiArea m_movementArea;

	void onNewBattlePhase(GameEvent gameEvent);
};