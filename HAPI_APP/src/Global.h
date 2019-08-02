#pragma once
#include <utility>
#include <string>
#include <SFML/Graphics.hpp>
#include <vector>

constexpr int INVALID_SHIP_ID = -1;
constexpr float DRAW_OFFSET_X{ 0 };
constexpr float DRAW_OFFSET_Y{ 8 };

const sf::Vector2u SCREEN_RESOLUTION{ 1680, 1050 };
const size_t MAX_SHIPS_PER_FACTION = 6;
const std::string DATA_DIRECTORY = "Data/";
const std::string SHIP_DATA_DIRECTORY = "Data/Ships/";
const std::string LEVEL_DATA_DIRECTORY = "Data/Levels/";
const sf::Vector2i MOUSE_POSITION_OFFSET{ 25, 45 };

enum class TileID
{
	Invalid = -1
};

enum FactionName
{
	eYellow = 0,
	eBlue = 1,
	eGreen = 2,
	eRed = 3,
	eTotal = eRed + 1
};

enum eDirection
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

enum OverWorldWindow
{
	eBattle = 0,
	eMainMenu,
	eShipSelection,
	eLevelSelection,
	ePlayerSelection,
	eUpgrade
};

enum BattlePhase
{
	Deployment = 0,
	Movement,
	Attack
};

enum eControllerType
{
	eLocalPlayer = 0,
	eAI,
	eRemotePlayer,
	None
};

enum eTileType
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

enum eShipSpriteFrame
{
	eMaxHealth = 0,
	eLowDamage,
	eHighDamage,
	eDead
};

enum class eShipType
{
	eFrigate,
	eTurtle,
	eFire,
	eSniper
};

enum eLightIntensity
{
	eMaximum = 0,
	//eHigh,
	//eLow,
	eMinimum
};

struct ShipOnTile
{
	ShipOnTile()
		: factionName(),
		shipID(INVALID_SHIP_ID)
	{}
	ShipOnTile(FactionName factionName, int shipID)
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

	FactionName factionName;
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
	eClientDisconnected
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

	int shipID;
	sf::Vector2i position;
	eDirection direction;
};

struct ServerMessageSpawnPosition
{
	ServerMessageSpawnPosition(FactionName factionName, int x, int y)
		:factionName(factionName),
		position(x, y)
	{}

	FactionName factionName;
	sf::Vector2i position;
};

struct ServerMessageExistingFaction
{
	ServerMessageExistingFaction(FactionName factionName, std::vector<eShipType>&& existingShips, bool AIControlled)
		: factionName(factionName),
		AIControlled(AIControlled),
		existingShips(std::move(existingShips))
	{}

	ServerMessageExistingFaction(FactionName factionName, const std::vector<eShipType>& existingShips, bool AIControlled)
		: factionName(factionName),
		AIControlled(AIControlled),
		existingShips(existingShips)
	{}

	FactionName factionName;
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

	ServerMessage(eMessageType type, FactionName factionName)
		: type(type),
		faction(factionName)
	{}

	ServerMessage(eMessageType type, FactionName factionName, std::vector<eShipType>&& shipsToAdd)
		: type(type),
		faction(factionName),
		shipsToAdd(std::move(shipsToAdd))
	{}

	eMessageType type;
	FactionName faction;
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

	Ray2D(int numX = 0, int numY = 0, eDirection direction = eNorth) : x(numX), y(numY), dir(direction) {}
	Ray2D(sf::Vector2i pair, eDirection direction = eNorth) : x(pair.x), y(pair.y), dir(direction) {}

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