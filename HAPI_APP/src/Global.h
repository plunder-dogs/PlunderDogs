#pragma once
#include <utility>

constexpr int INVALID_SHIP_ID = -1;
constexpr float DRAW_OFFSET_X{ 16 };
constexpr float DRAW_OFFSET_Y{ 32 };

enum FactionName
{
	Invalid = -1,
	eYellow = 0,
	eBlue,
	eGreen,
	eRed,
	MAX = 4
};

enum eDirection
{
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

enum ePlayerType
{
	eNone = 0,
	eHuman,
	eAI
};

enum ShipType
{
	eFrigate,
	eTurtle,
	eFire,
	eSniper
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

struct posi
{
	int x;
	int y;
	eDirection dir;

	posi(int numX = 0, int numY = 0, eDirection direction = eNorth) : x(numX), y(numY), dir(direction) {}
	posi(std::pair<int, int> pair, eDirection direction = eNorth) : x(pair.first), y(pair.second), dir(direction) {}

	std::pair<int, int> pair() { return { x, y }; }
	int dirDiff(const posi& compare)
	{
		int diff = std::abs(static_cast<int>(this->dir) - static_cast<int>(compare.dir));
		if (diff != 0)
			diff = (static_cast<int>(eDirection::Max) % diff) + 1;
		return diff;
	}
	//Comparison operators
	inline bool operator==(const posi& rhs) const
	{
		bool ans{ false };
		posi lhs = *this;
		if (lhs.x == rhs.x && lhs.y == rhs.y && lhs.dir == rhs.dir)
			ans = true;
		return ans;
	}
	inline bool operator!=(const posi& rhs) const { return !(*this == rhs); }
	//Assignment operators
	inline void operator+=(const posi& rhs) { this->x += rhs.x; this->y += rhs.y; }
	inline void operator-=(const posi& rhs) { this->x -= rhs.x; this->y -= rhs.y; }
	inline void operator*=(const posi& rhs) { this->x *= rhs.x; this->y *= rhs.y; }
	inline void operator/=(const posi& rhs) { this->x /= rhs.x; this->y /= rhs.y; }
	inline void operator%=(const posi& rhs) { this->x %= rhs.x; this->y %= rhs.y; }
	//Maths operators
	inline posi& operator+(const posi& rhs) const { return posi(this->x + rhs.x, this->y + rhs.y, this->dir); }
	inline posi& operator-(const posi& rhs) const { return posi(this->x - rhs.x, this->y - rhs.y, this->dir); }
	inline posi& operator*(const posi& rhs) const { return posi(this->x * rhs.x, this->y * rhs.y, this->dir); }
	inline posi& operator/(const posi& rhs) const { return posi(this->x / rhs.x, this->y / rhs.y, this->dir); }
	inline posi& operator%(const posi& rhs) const { return posi(this->x % rhs.x, this->y % rhs.y, this->dir); }
};