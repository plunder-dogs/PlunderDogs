#pragma once
#include <utility>
#include <string>
#include <SFML/Graphics.hpp>
#include <vector>

constexpr int INVALID_SHIP_ID = -1;
constexpr float DRAW_OFFSET_X{ 0 };
constexpr float DRAW_OFFSET_Y{ 8 };

const size_t MAX_SHIPS_PER_FACTION = 6;
const sf::Vector2i MOUSE_POSITION_OFFSET{ 25, 45 };

enum class eTileID
{
	Invalid = -1
};

enum class eFactionName
{
	eYellow = 0,
	eBlue = 1,
	eGreen = 2,
	eRed = 3,
	eTotal = eRed + 1
};

enum class eDirection
{
	eInvalid = -1,
	eNorth,
	eNorthEast,
	eSouthEast,
	eSouth,
	eSouthWest,
	eNorthWest,
	Max = eNorthWest
};

enum class eBattlePhase
{
	Deployment = 0,
	Movement,
	Attack
};

enum class eFactionControllerType
{
	eLocalPlayer = 0,
	eAI,
	eRemotePlayer,
	None
};

enum class eTileType
{
	eGrass = 0,
	eSparseForest,
	eForest,
	eFoothills,
	eWoodedFoothills,
	eMountain,
	eSea,
	eOcean,
	eGrasslandTown,
	eWalledGrasslandTown,
	eStoneGrasslandTown,
	eFarm,
	eWoodedSwamp,
	eSwampPools,
	eSwamp,
	eSwampWater,
	eSnow,
	eSparseSnowForest,
	eSnowForest,
	eSnowFoothills,
	eSnowWoodedFoothills,
	eIceburgs,
	eSnowTown,
	eSnowCastle,
	eSand,
	eSandFoothills,
	eSandDunes,
	eMesa,
	eOasis,
	eSandTown,
	eWalledSandTown,
	eJungle,
	eLeftPort,
	eRightPort,
	eLighthouse,
	eGrasslandRuin,
	eSwampRuins
};

enum class eShipType
{
	eFrigate,
	eTurtle,
	eFire,
	eSniper
};

struct ShipOnTile
{
	ShipOnTile()
		: factionName(),
		shipID(INVALID_SHIP_ID)
	{}
	ShipOnTile(eFactionName factionName, int shipID)
		: factionName(factionName),
		shipID(shipID)
	{}

	bool operator==(const ShipOnTile& orig)
	{
		return (factionName == orig.factionName && shipID == orig.shipID);
	}

	bool isValid() const
	{
		return (shipID != INVALID_SHIP_ID);
	}

	void clear()
	{
		shipID = INVALID_SHIP_ID;
	}

	eFactionName factionName;
	int shipID;
};

enum class eMessageType
{
	eEstablishConnection = 0,
	eRefuseConnection,
	eNewPlayer,
	ePlayerReady,
	eStartOnlineGame,
	eDeployShipAtPosition,
	eMoveShipToPosition,
	eAttackShipAtPosition,
	eDisconnect,
	eClientDisconnected,
	eRemotePlayerReady
};

struct ServerMessageShipAction
{
	ServerMessageShipAction(int shipID, int x, int y)
		: shipID(shipID),
		position(x, y),
		direction(eDirection::eNorth)
	{}

	ServerMessageShipAction(int shipID, int x, int y, eDirection direction)
		: shipID(shipID),
		position(x, y),
		direction(direction)
	{}

	ServerMessageShipAction(int x, int y, eDirection direction)
		: shipID(-1),
		position(x, y),
		direction(direction)
	{}

	int shipID;
	sf::Vector2i position;
	eDirection direction;
};

struct ServerMessageSpawnPosition
{
	ServerMessageSpawnPosition(eFactionName factionName, int x, int y)
		:factionName(factionName),
		position(x, y)
	{}

	eFactionName factionName;
	sf::Vector2i position;
};

struct ServerMessageExistingFaction
{
	ServerMessageExistingFaction(eFactionName factionName, std::vector<eShipType>&& existingShips, bool AIControlled, bool ready)
		: factionName(factionName),
		ready(ready),
		AIControlled(AIControlled),
		existingShips(std::move(existingShips))
	{}

	ServerMessageExistingFaction(eFactionName factionName, const std::vector<eShipType>& existingShips, bool AIControlled, bool ready)
		: factionName(factionName),
		ready(ready),
		AIControlled(AIControlled),
		existingShips(existingShips)
	{}

	eFactionName factionName;
	bool ready;
	bool AIControlled;
	std::vector<eShipType> existingShips;
};

struct ServerMessage
{
	ServerMessage()
	{}

	ServerMessage(eMessageType type)
		: type(type)
	{}

	ServerMessage(eMessageType type, eFactionName factionName)
		: type(type),
		faction(factionName)
	{}

	ServerMessage(eMessageType type, eFactionName factionName, std::vector<eShipType>&& shipsToAdd)
		: type(type),
		faction(factionName),
		shipsToAdd(std::move(shipsToAdd))
	{}

	eMessageType type;
	eFactionName faction;
	std::string levelName;
	std::vector<eShipType> shipsToAdd;
	std::vector<ServerMessageShipAction> shipActions;
	std::vector<ServerMessageSpawnPosition> spawnPositions;
	std::vector<ServerMessageExistingFaction> existingFactions;
};

struct Ray2D
{
	int x;
	int y;
	eDirection dir;

	Ray2D(int numX = 0, int numY = 0, eDirection direction = eDirection::eNorth) : x(numX), y(numY), dir(direction) {}
	Ray2D(sf::Vector2i pair, eDirection direction = eDirection::eNorth) : x(pair.x), y(pair.y), dir(direction) {}

	sf::Vector2i pair() const { return { x, y }; }
	int dirDiff(const Ray2D& compare)
	{
		int diff = std::abs(static_cast<int>(this->dir) - static_cast<int>(compare.dir));
		if (diff != 0)
			diff = (static_cast<int>(eDirection::Max) % diff) + 1;
		return diff;
	}
	//Comparison operators
	inline bool operator==(const Ray2D& rhs) const
	{
		bool ans{ false };
		Ray2D lhs = *this;
		if (lhs.x == rhs.x && lhs.y == rhs.y && lhs.dir == rhs.dir)
			ans = true;
		return ans;
	}
	inline bool operator!=(const Ray2D& rhs) const { return !(*this == rhs); }
	//Assignment operators
	inline void operator+=(const Ray2D& rhs) { this->x += rhs.x; this->y += rhs.y; }
	inline void operator-=(const Ray2D& rhs) { this->x -= rhs.x; this->y -= rhs.y; }
	inline void operator*=(const Ray2D& rhs) { this->x *= rhs.x; this->y *= rhs.y; }
	inline void operator/=(const Ray2D& rhs) { this->x /= rhs.x; this->y /= rhs.y; }
	inline void operator%=(const Ray2D& rhs) { this->x %= rhs.x; this->y %= rhs.y; }
	//Maths operators
	inline Ray2D& operator+(const Ray2D& rhs) const { return Ray2D(this->x + rhs.x, this->y + rhs.y, this->dir); }
	inline Ray2D& operator-(const Ray2D& rhs) const { return Ray2D(this->x - rhs.x, this->y - rhs.y, this->dir); }
	inline Ray2D& operator*(const Ray2D& rhs) const { return Ray2D(this->x * rhs.x, this->y * rhs.y, this->dir); }
	inline Ray2D& operator/(const Ray2D& rhs) const { return Ray2D(this->x / rhs.x, this->y / rhs.y, this->dir); }
	inline Ray2D& operator%(const Ray2D& rhs) const { return Ray2D(this->x % rhs.x, this->y % rhs.y, this->dir); }
};