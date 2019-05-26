#pragma once

#include <HAPISprites_lib.h>
#include <HAPISprites_UI.h>
#include <memory>
#include "Timer.h"
#include "Global.h"
#include "SpriteToggleVisibility.h"

//TODO: temp
constexpr float MOVEMENT_ANIMATION_TIME(0.35f);

struct Tile;
struct Weapons;
class Map;
class Battle;
class Ship
{
	class MovementPath
	{
	public:
		MovementPath();

		void render(const Map& map) const;
		int generatePath(const Map& map, const Tile& source, const Tile& destination);
		void eraseNode(std::pair<int, int> position, const Map& map);
		void clearPath();
		void setNodePosition(int i, std::pair<int, int> newPosition) { m_movementPath[i].m_position = newPosition; }

		std::pair<int, int> getFinalNode() const;
	private:
		
		unsigned int getDirectionCost(int currentDirection, int newDirection);
	};

public:
	Ship(FactionName playerName, eShipType shipType);
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

	void update(float deltaTime, const Map& map);
	void render(const Map& map) const;

	void setDeploymentPosition(std::pair<int, int> position, const Battle& battle);
	void deployAtPosition(std::pair<int, int> position, Battle& battle, eDirection startingDirection = eDirection::eNorth);

	std::vector<posi> generateMovementArea(const Map& map, float movement) const;
	int generateMovementPath(const Map& map, const Tile& source, const Tile& destination);
	void clearMovementPath();
	std::pair<int, int> getEndOfPath();

	bool moveEntity(Map& map, const Tile& tile);
	bool moveEntity(Map& map, const Tile& tile, eDirection endDirection);
	void takeDamage(int damageAmount, FactionName entityFaction);
	void fireWeapon();
	void setDestination();
	void onNewTurn();
	
	void enableAction();
	void disableAction();

private:
	const FactionName m_factionName;
	const eShipType m_shipType;

	std::pair<int, int> m_currentPosition;
	std::queue<posi> m_pathToTile;
	Timer m_movementTimer;
	//MovementPath m_movementPath;
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

	void render(const Map& map) const;
	void eraseMovementPathNode(std::pair<int, int> position, const Map& map); 
	void setMovementPathNodePosition(int i, std::pair<int, int> newPosition) { m_movementPath[i].m_position = newPosition; }
	std::pair<int, int> getMovementPathEndPosition() const;

private:

	unsigned int getDirectionCost(int currentDirection, int newDirection);

	void handleRotation();
};

struct SpawnNode
{
	SpawnNode(FactionName factionName, std::pair<int, int> position, const Map& map);

	void render(const Map& map) const;

	std::pair<int, int> m_position;
	std::unique_ptr<Sprite> m_sprite;
};

struct Player
{
	Player(FactionName name, ePlayerType playerType);
	
	void render(const Map& map) const;
	void createSpawnArea(Map& map);
	void onNewTurn();

	std::vector<Ship> m_ships;
	const FactionName m_factionName;
	const ePlayerType m_playerType;
	bool m_eliminated;
	Ship* m_shipToDeploy;
	std::vector<SpawnNode> m_spawnArea;
};