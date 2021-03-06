#include "PathFinding.h"
#include "Map.h"
#include <assert.h>
#include "TileArea.h"

Ray2D nextTile(const Ray2D& currentTile, int mapWidth, int maxSize);
Ray2D turnLeft(const Ray2D& currentTile);
Ray2D turnRight(const Ray2D& currentTile);

void PathFinding::clear()
{
	m_byteData.clear();
	m_tileData.clear();
}

void PathFinding::loadTileData(const Map & map)
{
	assert(!map.getData().empty());
	
	assert(m_tileData.empty());
	m_tileData.reserve(map.getDimensions().x * map.getDimensions().y);
	for (const Tile& tile : map.getData())
	{
		bool tileTraversable = false;
		if ((tile.m_type == eTileType::eSea || tile.m_type == eTileType::eOcean) && !tile.isShipOnTile())
		{
			tileTraversable = true;
		}

		m_tileData.emplace_back(tileTraversable);
	}

	assert(m_byteData.empty());
	m_byteData.reserve(map.getDimensions().x * map.getDimensions().y);
	for (const Tile& tile : map.getData())
	{
		bool tileTraversable = (tile.m_type == eTileType::eSea || tile.m_type == eTileType::eOcean) && !tile.m_shipOnTile.isValid();
		m_byteData.emplace_back(tileTraversable);
	}
}

void PathFinding::findPath(Ray2DArea& tileArea, const Map & map, Ray2D startPos, Ray2D endPos, float maxMovement)
{
	if (!map.getTile(startPos) || !map.getTile(endPos) ||
		map.getTile(endPos)->m_shipOnTile.isValid() ||
		(map.getTile(endPos)->m_type != eTileType::eSea && map.getTile(endPos)->m_type != eTileType::eOcean))
	{
		return;
	}

	resetTileData(map);
	std::queue<std::pair<Ray2D, float>> exploreQueue;
	//Add first element and set it to explored
	exploreQueue.emplace(startPos, maxMovement);
	accessTileData(startPos, map.getDimensions().x).m_neighbours[static_cast<int>(startPos.dir)] = startPos;//Yes, it's set = itself as it's the root note
	//Start recursion
	Ray2D trace{ NO_TILE };
	while (!exploreQueue.empty())
	{
		if (pathExplorer(trace, exploreQueue, endPos, map.getWindDirection(), map.getWindStrength(), map.getDimensions().x))
			break;
	}

	if (trace == NO_TILE)
	{
		return;
	}

	tileArea.clearTileArea();
	//Trace path back from destination via parents
	while (trace != startPos)
	{
		tileArea.m_tileArea.push_back(trace);
		trace = accessTileData(trace, map.getDimensions().x).m_neighbours[static_cast<int>(trace.dir)];
	}
	std::reverse(tileArea.m_tileArea.begin(), tileArea.m_tileArea.end());
}

std::queue<Ray2D> PathFinding::findPath(const Map& map, Ray2D startPos, Ray2D endPos, float maxMovement)
{
	if (!map.getTile(startPos) || !map.getTile(endPos) ||
		map.getTile(endPos)->m_shipOnTile.isValid() ||
		(map.getTile(endPos)->m_type != eTileType::eSea && map.getTile(endPos)->m_type != eTileType::eOcean))
		return std::queue<Ray2D>();

	resetTileData(map);
	std::queue<std::pair<Ray2D, float>> exploreQueue;
	//Add first element and set it to explored
	exploreQueue.emplace(startPos, maxMovement);
	accessTileData(startPos, map.getDimensions().x).m_neighbours[static_cast<int>(startPos.dir)] = startPos;//Yes, it's set = itself as it's the root note
	//Start recursion
	Ray2D trace{ NO_TILE };
	while (!exploreQueue.empty())
	{
		if (pathExplorer(trace, exploreQueue, endPos, map.getWindDirection(), map.getWindStrength(), map.getDimensions().x))
			break;
	}
	if (trace == NO_TILE)
		return std::queue<Ray2D>();
	
	//Trace path back from destination via parents
	std::vector<Ray2D> pathToTile;
	pathToTile.reserve(25);
	while (trace != startPos)
	{
		pathToTile.emplace_back(trace);
		trace = accessTileData(trace, map.getDimensions().x).m_neighbours[static_cast<int>(trace.dir)];
	}

	std::reverse(pathToTile.begin(), pathToTile.end());

	//Invert for convenience and fetch tile* for each address
	std::queue<Ray2D> finalPath;
	for (int i = pathToTile.size() - 1; i >= 0; i--)
	{
		finalPath.emplace(pathToTile[i]);
	}
	return finalPath;
}

std::vector<Ray2D> PathFinding::findArea(const Map & map, Ray2D startPos, float maxMovement)
{
	if (!map.getTile(startPos))
		return std::vector<Ray2D>();

	resetByteData(map);
	//boolMap exploreArea(map);
	std::queue<std::pair<Ray2D, float>> exploreQueue;
	//Add first element and set it to explored
	exploreQueue.emplace(startPos, maxMovement);
	//accessByteData(
	accessByteData(startPos, map.getDimensions().x).setDir(static_cast<int>(startPos.dir), true);//Yes, it's set = itself as it's the root note
	//Start recursion
	int areaSize{ 1 };
	while (!exploreQueue.empty())
	{
		if (areaExplorer(exploreQueue, map.getWindDirection(), map.getWindStrength(), map.getDimensions().x))
			areaSize++;
	}
	//Iterate through exploreArea and pushback to the return vector
	std::vector<Ray2D> allowedArea;
	allowedArea.reserve(areaSize);
	for (int i = 0; i < m_byteData.size(); i++)
	{
		if (m_byteData[i].encountered())
			allowedArea.emplace_back(i % map.getDimensions().x, i / map.getDimensions().x);
	}
	return allowedArea;
}

void PathFinding::findArea(std::vector<const Tile*>& tileArea, const Map & map, Ray2D startPos, float maxMovement)
{
	if (!map.getTile(startPos))
	{
		return;
	}

	resetByteData(map);
	//boolMap exploreArea(map);
	std::queue<std::pair<Ray2D, float>> exploreQueue;
	//Add first element and set it to explored
	exploreQueue.emplace(startPos, maxMovement);
	accessByteData(startPos, map.getDimensions().x).setDir(static_cast<int>(startPos.dir), true);//Yes, it's set = itself as it's the root note
	//Start recursion
	int areaSize{ 1 };
	while (!exploreQueue.empty())
	{
		if (areaExplorer(exploreQueue, map.getWindDirection(), map.getWindStrength(), map.getDimensions().x))
			areaSize++;
	}

	//Iterate through exploreArea and pushback to the return vector
	for (int i = 0; i < m_byteData.size(); i++)
	{
		if (m_byteData[i].encountered())
		{
			tileArea.push_back(map.getTile(Ray2D(i % map.getDimensions().x, i / map.getDimensions().x)));
		}
	}
}

PathFinding::TileData & PathFinding::accessTileData(Ray2D tile, int mapWidth)
{
	assert(tile.x + tile.y * mapWidth < m_tileData.size());
	return m_tileData[tile.x + tile.y * mapWidth];
}

ByteStore & PathFinding::accessByteData(Ray2D tile, int mapWidth)
{
	assert(tile.x + tile.y * mapWidth < m_byteData.size());
	return m_byteData[tile.x + tile.y * mapWidth];
}

bool PathFinding::pathExplorer(Ray2D & trace, std::queue<std::pair<Ray2D, float>>& queue, Ray2D destination, const eDirection windDirection, 
	const float windStrength, int mapWidth)
{
	//Dequeue a tile
	Ray2D tile = queue.front().first;
	float tether = queue.front().second;
	queue.pop();
	//Check if there's enough movement to do check other tiles
	if (tether >= 0.0f)
	{
		//Check if this is the destination
		if (tile.pair() == destination.pair())
		{
			trace = tile;
			return true;
		}
		//Check if any movements are unexplored and movable, if so set those to true and set their parent to this tile
		//Then enqueue left, right, and forward as appropriate:
		
		//Left
		Ray2D queueTile = turnLeft(tile);
		if (accessTileData(queueTile, mapWidth).m_neighbours[static_cast<int>(queueTile.dir)] == NO_TILE)
		{
			accessTileData(queueTile, mapWidth).m_neighbours[static_cast<int>(queueTile.dir)] = tile;
			queue.emplace(queueTile, tether - 1);
		}
		//Right
		queueTile = turnRight(tile);
		if (accessTileData(queueTile, mapWidth).m_neighbours[static_cast<int>(queueTile.dir)] == NO_TILE)
		{
			accessTileData(queueTile, mapWidth).m_neighbours[static_cast<int>(queueTile.dir)] = tile;
			queue.emplace(queueTile, tether - 1);
		}
		//Forward
		queueTile = nextTile(tile, mapWidth, m_tileData.size());
		if (queueTile != NO_TILE &&
			accessTileData(queueTile, mapWidth).m_neighbours[static_cast<int>(queueTile.dir)] == NO_TILE &&
			accessTileData(queueTile, mapWidth).m_traversable)
		{
			accessTileData(queueTile, mapWidth).m_neighbours[static_cast<int>(queueTile.dir)] = tile;
			if (queueTile.dir == windDirection)
				queue.emplace(queueTile, tether - (1.0f - windStrength));
			else
				queue.emplace(queueTile, tether - 1);
		}
	}

	return false;
}

bool PathFinding::areaExplorer(std::queue<std::pair<Ray2D, float>>& queue, eDirection windDirection, float windStrength, int mapWidth)
{
	//Dequeue a tile
	Ray2D tile = queue.front().first;
	float tether = queue.front().second;
	queue.pop();
	bool output = false;
	//Check if there's enough movement to do check other tiles
	if (tether >= 0.0f)
	{
		//Set tile to show in result
		accessByteData(tile, mapWidth).setEncountered(true);
		//Check if any movements are unexplored and movable, if so set those to true and set their parent to this tile
		//Then enqueue left, right, and forward as appropriate:
		//Left
		Ray2D queueTile = turnLeft(tile);
		if (!accessByteData(queueTile, mapWidth).getDir(static_cast<int>(queueTile.dir)))
		{
			accessByteData(queueTile, mapWidth).setDir(static_cast<int>(queueTile.dir), true);
			queue.emplace(queueTile, tether - 1);
		}
		//Right
		queueTile = turnRight(tile);
		if (!accessByteData(queueTile, mapWidth).getDir(static_cast<int>(queueTile.dir)))
		{
			accessByteData(queueTile, mapWidth).setDir(static_cast<int>(queueTile.dir), true);
			queue.emplace(queueTile, tether - 1);
		}
		//Forward
		queueTile = nextTile(tile, mapWidth, m_byteData.size());
		if (queueTile != NO_TILE &&
			!accessByteData(queueTile, mapWidth).getDir(static_cast<int>(queueTile.dir)) &&
			accessByteData(queueTile, mapWidth).traversable())
		{
			output = true;
			accessByteData(queueTile, mapWidth).setDir(static_cast<int>(queueTile.dir), true);
			if (queueTile.dir == windDirection)
				queue.emplace(queueTile, tether - (1.0f - windStrength));
			else
				queue.emplace(queueTile, tether - 1);
		}
	}
	//If queue is not empty run algorithm again
	return output;
}

Ray2D nextTile(const Ray2D & currentTile, int mapWidth, int maxSize)
{
	int x = currentTile.x;
	int y = currentTile.y;
	Ray2D nextAddress = currentTile;

	if (x & 1)//odd
	{
		switch (currentTile.dir)
		{
		case eDirection::eNorth:
			nextAddress.x = x;
			nextAddress.y = y - 1;
			break;
		case eDirection::eNorthEast:
			nextAddress.x = x + 1;
			nextAddress.y = y - 1;
			break;
		case eDirection::eSouthEast:
			nextAddress.x = x + 1;
			nextAddress.y = y;
			break;
		case eDirection::eSouth:
			nextAddress.x = x;
			nextAddress.y = y + 1;
			break;
		case eDirection::eSouthWest:
			nextAddress.x = x - 1;
			nextAddress.y = y;
			break;
		case eDirection::eNorthWest:
			nextAddress.x = x - 1;
			nextAddress.y = y - 1;
			break;
		}
	}
	else//even
	{
		switch (currentTile.dir)
		{
		case eDirection::eNorth:
			nextAddress.x = x;
			nextAddress.y = y - 1;
			break;
		case eDirection::eNorthEast:
			nextAddress.x = x + 1;
			nextAddress.y = y;
			break;
		case eDirection::eSouthEast:
			nextAddress.x = x + 1;
			nextAddress.y = y + 1;
			break;
		case eDirection::eSouth:
			nextAddress.x = x;
			nextAddress.y = y + 1;
			break;
		case eDirection::eSouthWest:
			nextAddress.x = x - 1;
			nextAddress.y = y + 1;
			break;
		case eDirection::eNorthWest:
			nextAddress.x = x - 1;
			nextAddress.y = y;
			break;
		}
	}

	//Bounds checking
	if (nextAddress.x < 0 || nextAddress.y < 0 || nextAddress.x >= mapWidth || (nextAddress.x + nextAddress.y * mapWidth) >= maxSize)
	{
		nextAddress = NO_TILE;
	}
		
	return nextAddress;
}

Ray2D turnLeft(const Ray2D & currentTile) 
{
	Ray2D nextTile = currentTile;
	switch (currentTile.dir)
	{
	case eDirection::eNorth:
		nextTile.dir = eDirection::eNorthWest;
		break;
	case eDirection::eNorthEast:
		nextTile.dir = eDirection::eNorth;
		break;
	case eDirection::eSouthEast:
		nextTile.dir = eDirection::eNorthEast;
		break;
	case eDirection::eSouth:
		nextTile.dir = eDirection::eSouthEast;
		break;
	case eDirection::eSouthWest:
		nextTile.dir = eDirection::eSouth;
		break;
	case eDirection::eNorthWest:
		nextTile.dir = eDirection::eSouthWest;
		break;
	}

	return nextTile;
}

Ray2D turnRight(const Ray2D & currentTile)
{
	Ray2D nextTile = currentTile;
	switch (currentTile.dir)
	{
	case eDirection::eNorth:
		nextTile.dir = eDirection::eNorthEast;
		break;
	case eDirection::eNorthEast:
		nextTile.dir = eDirection::eSouthEast;
		break;
	case eDirection::eSouthEast:
		nextTile.dir = eDirection::eSouth;
		break;
	case eDirection::eSouth:
		nextTile.dir = eDirection::eSouthWest;
		break;
	case eDirection::eSouthWest:
		nextTile.dir = eDirection::eNorthWest;
		break;
	case eDirection::eNorthWest:
		nextTile.dir = eDirection::eNorth;
		break;
	}

	return nextTile;
}

void PathFinding::resetByteData(const Map & map)
{
	assert(m_byteData.size() == map.getData().size());

	for (int i = 0; i < map.getData().size(); ++i)
	{
		bool tileTraversable = false;
		if ((map.getData()[i].m_type == eTileType::eSea || map.getData()[i].m_type == eTileType::eOcean)
			&& !map.getData()[i].isShipOnTile())
		{
			tileTraversable = true;
		}

		m_byteData[i].reset(tileTraversable);
	}
}

void PathFinding::resetTileData(const Map & map)
{
	assert(m_tileData.size() == map.getData().size());

	for (int i = 0; i < map.getData().size(); ++i)
	{
		bool tileTraversable = false;
		if ((map.getData()[i].m_type == eTileType::eSea || map.getData()[i].m_type == eTileType::eOcean) && !map.getData()[i].isShipOnTile())
		{
			tileTraversable = true;
		}

		m_tileData[i].m_traversable = tileTraversable;
		m_tileData[i].resetNeighbours();
	}
}