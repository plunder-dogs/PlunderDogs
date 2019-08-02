#pragma once
#include <vector>
#include <queue>
#include "Global.h"

//A 1 byte store for all the data, very compact but annoying to use
struct byteStore
{
	unsigned char byte;
	byteStore(bool traversable = false) : byte(0) { if (traversable) byte = (1 << 7); }
	//First bit
	bool traversable() { return byte & 128; }
	void setTraversable(bool istraversable)
	{
		if (istraversable)
			byte |= (1 << 7);
		else
			byte &= ~(1 << 7);
	}
	//y bit
	bool encountered() { return byte & 64; }
	void setEncountered(bool isEncountered)
	{
		if (isEncountered)
			byte |= (1 << 6);
		else
			byte &= ~(1 << 6);
	}
	//Available directions are between 0 and 5 inclusive. No checking for speed so be careful
	bool getDir(int dir) { return byte & static_cast<int>(std::pow(2, dir)); }
	void setDir(int dir, bool set)
	{
		if (set)
			byte |= (1 << dir);
		else
			byte &= ~(1 << dir);
	}
};

struct Tile;
class Map;
class PathFinding
{
	#define NO_TILE Ray2D(static_cast<int>(TileID::Invalid), static_cast<int>(TileID::Invalid), eDirection::eInvalid)

	struct TileData
	{
		TileData(bool traversable, bool occupied) :
			isTraversable(traversable), isOccupied(occupied),
			parent{ NO_TILE, NO_TILE, NO_TILE, NO_TILE, NO_TILE, NO_TILE } {}

		const bool isTraversable;
		bool isOccupied;
		//The node that was first used to access the corresponding direction during the BFS
		//One for each direction in order
		Ray2D parent[6];
	};

public:
	static PathFinding& getInstance()
	{
		static PathFinding instance;
		return instance;
	}

	void loadTileData(const Map& map);

	//For finding the optimal path for a ship, returns an empty queue if it can't be reached
	std::queue<Ray2D> findPath(const Map& map, Ray2D startPos, Ray2D endPos, float maxMovement = 10);

	//For finding the possible movement area of a ship
	std::vector<Ray2D> findArea(const Map& map, Ray2D startPos, float maxMovement = 10);
	void findArea(std::vector<const Tile*>& tileArea, const Map& map, Ray2D startPos, float maxMovement = 10);


private:
	std::vector<TileData> m_tileData;
	std::vector<byteStore> m_byteData;

	TileData& accessTileData(Ray2D tile, int mapWidth);
	byteStore& accessByteData(Ray2D tile, int mapWidth);

	bool pathExplorer(Ray2D& finalPoint, std::queue<std::pair<Ray2D, float>>& queue, Ray2D destination, eDirection windDirection, float windStrength,
		int mapWidth);

	bool areaExplorer(std::queue<std::pair<Ray2D, float>>& queue, eDirection windDirection, float windStrength, int mapWidth);

	Ray2D nextTile(const Ray2D& currentTile, int mapWidth, int maxSize) const;
	Ray2D turnLeft(const Ray2D& currentTile) const;
	Ray2D turnRight(const Ray2D& currentTile) const;

	void resetByteData(const Map& map);
	void resetTileData(const Map& map);
};