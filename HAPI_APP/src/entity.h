#pragma once

#include <HAPISprites_lib.h>
#include <HAPISprites_UI.h>
#include <memory>
#include "Timer.h"
#include "Global.h"

enum class ShipType
{
	eFrigate,
	eTurtle,
	eFire,
	eSniper
};

struct Tile;
struct Weapons;
class Map;

struct Ship
{
	struct ActionSprite
	{
		ActionSprite(FactionName factionName);

		void render(const Map& map, std::pair<int, int> currentEntityPosition) const;

		std::unique_ptr<Sprite> sprite;
		bool active;
	};

	class MovementPath
	{
		struct PathNode
		{
			PathNode();

			std::unique_ptr<Sprite> sprite;
			bool activate;
			std::pair<int, int> m_position;
		};

	public:
		MovementPath();

		void render(const Map& map) const;
		int generatePath(const Map& map, const Tile& source, const Tile& destination);
		void eraseNode(std::pair<int, int> position, const Map& map);
		void clearPath();
		void setNodePosition(int i, std::pair<int, int> newPosition) { m_movementPath[i].m_position = newPosition; }

		std::pair<int, int> getFinalNode() const;
	private:
		std::vector<PathNode> m_movementPath;
		unsigned int getDirectionCost(int currentDirection, int newDirection);
	};

public:
	Ship(std::pair<int, int> startingPosition, const ShipGlobalProperties& entityProperties, Map& map, FactionName playerName, 
		eDirection startingDirection = eNorth, int health, int damage, int range, eWeaponType weaponType);
	~Ship();

	ShipGlobalProperties& getProperties();
	FactionName getFactionName() const;
	eDirection getCurrentDirection() const;
	std::pair<int, int> getCurrentPosition() const;
	bool isWeaponFired() const;
	bool isDead() const;
	bool isMovingToDestination() const;
	bool isDestinationSet() const;

	void update(float deltaTime, const Map& map, ShipGlobalProperties& entityProperties);
	void render(std::shared_ptr<HAPISPACE::Sprite>& sprite, const Map& map);
	void renderPath(const Map & map);

	std::vector<posi> generateMovementArea(const Map& map, float movement) const;
	int generateMovementGraph(const Map& map, const Tile& source, const Tile& destination);
	void clearMovementPath();
	std::pair<int, int> getEndOfPath();

	bool moveEntity(Map& map, const Tile& tile);
	bool moveEntity(Map& map, const Tile& tile, eDirection endDirection);
	void takeDamage(ShipGlobalProperties& entityProperties, int damageAmount, FactionName entityFaction);
	void fireWeapon();
	void setDestination();
	void onNewTurn();
	
	void enableAction();
	void disableAction();

private:
	const FactionName m_factionName;
	ShipProperties m_entityProperties;
	std::pair<int, int> m_currentPosition;
	std::queue<posi> m_pathToTile;
	Timer m_movementTimer;
	MovementPath m_movementPath;
	int m_movementPathSize;
	eDirection m_currentDirection;
	bool m_weaponFired;
	//bool m_movedToDestination;
	bool m_isDead;
	ActionSprite m_actionSprite;
	bool m_movingToDestination;
	bool m_destinationSet;
	int m_health;
	int m_damage;
	int m_range;
	eWeaponType m_weaponType;
	std::unique_ptr<Sprite> m_sprite;

	void handleRotation(ShipGlobalProperties& entityProperties);
};

//struct Player
//{
//	Player(FactionName name, ePlayerType playerType);
//
//	std::vector<ShipGlobalProperties> m_entities;
//	std::vector<ShipGlobalProperties*> m_selectedEntities;
//	FactionName m_factionName;
//	ePlayerType m_type;
//};

struct BattlePlayer
{
	BattlePlayer(FactionName name, std::pair<int, int> spawnPosition, ePlayerType playerType);

	std::vector<std::unique_ptr<Ship>> m_entities;
	const FactionName m_factionName;
	const ePlayerType m_playerType;
	const std::pair<int, int> m_spawnPosition;
	bool m_eliminated;
	std::deque<ShipGlobalProperties*> m_shipsToDeploy;
};