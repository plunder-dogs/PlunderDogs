#include "PathFinding.h"
#include "Map.h"
#include <assert.h>

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
		bool tileTraversable = (tile.m_type == eSea || tile.m_type == eOcean) && !tile.m_shipOnTile.isValid();
		m_byteData.emplace_back(tileTraversable);
	}
}

void PathFinding::findPath(std::deque<Ray2D>& tileArea, const Map & map, Ray2D startPos, Ray2D endPos, float maxMovement)
{
	if (!map.getTile(startPos) || !map.getTile(endPos) ||
		map.getTile(endPos)->m_shipOnTile.isValid() ||
		(map.getTile(endPos)->m_type != eSea && map.getTile(endPos)->m_type != eOcean))
	{
		return;
	}

	resetTileData(map);
	std::queue<std::pair<Ray2D, float>> exploreQueue;
	//Add first element and set it to explored
	exploreQueue.emplace(startPos, maxMovement);
	accessTileData(startPos, map.getDimensions().x).m_parent[startPos.dir] = startPos;//Yes, it's set = itself as it's the root note
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

	tileArea.clear();
	//Trace path back from destination via parents
	while (trace != startPos)
	{
		tileArea.push_back(trace);
		trace = accessTileData(trace, map.getDimensions().x).m_parent[trace.dir];
	}
	std::reverse(tileArea.begin(), tileArea.end());
}

std::queue<Ray2D> PathFinding::findPath(const Map& map, Ray2D startPos, Ray2D endPos, float maxMovement)
{
	if (!map.getTile(startPos) || !map.getTile(endPos) ||
		map.getTile(endPos)->m_shipOnTile.isValid() ||
		(map.getTile(endPos)->m_type != eSea && map.getTile(endPos)->m_type != eOcean))
		return std::queue<Ray2D>();

	resetTileData(map);
	std::queue<std::pair<Ray2D, float>> exploreQueue;
	//Add first element and set it to explored
	exploreQueue.emplace(startPos, maxMovement);
	accessTileData(startPos, map.getDimensions().x).m_parent[startPos.dir] = startPos;//Yes, it's set = itself as it's the root note
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
		trace = accessTileData(trace, map.getDimensions().x).m_parent[trace.dir];
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
	accessByteData(startPos, map.getDimensions().x).setDir(startPos.dir, true);//Yes, it's set = itself as it's the root note
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
	accessByteData(startPos, map.getDimensions().x).setDir(startPos.dir, true);//Yes, it's set = itself as it's the root note
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
		if (accessTileData(queueTile, mapWidth).m_parent[queueTile.dir] == NO_TILE)
		{
			accessTileData(queueTile, mapWidth).m_parent[queueTile.dir] = tile;
			queue.emplace(queueTile, tether - 1);
		}
		//Right
		queueTile = turnRight(tile);
		if (accessTileData(queueTile, mapWidth).m_parent[queueTile.dir] == NO_TILE)
		{
			accessTileData(queueTile, mapWidth).m_parent[queueTile.dir] = tile;
			queue.emplace(queueTile, tether - 1);
		}
		//Forward
		queueTile = nextTile(tile, mapWidth, m_tileData.size());
		if (queueTile != NO_TILE &&
			accessTileData(queueTile, mapWidth).m_parent[queueTile.dir] == NO_TILE &&
			accessTileData(queueTile, mapWidth).m_traversable)
		{
			accessTileData(queueTile, mapWidth).m_parent[queueTile.dir] = tile;
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
		if (!accessByteData(queueTile, mapWidth).getDir(queueTile.dir))
		{
			accessByteData(queueTile, mapWidth).setDir(queueTile.dir, true);
			queue.emplace(queueTile, tether - 1);
		}
		//Right
		queueTile = turnRight(tile);
		if (!accessByteData(queueTile, mapWidth).getDir(queueTile.dir))
		{
			accessByteData(queueTile, mapWidth).setDir(queueTile.dir, true);
			queue.emplace(queueTile, tether - 1);
		}
		//Forward
		queueTile = nextTile(tile, mapWidth, m_byteData.size());
		if (queueTile != NO_TILE &&
			!accessByteData(queueTile, mapWidth).getDir(queueTile.dir) &&
			accessByteData(queueTile, mapWidth).traversable())
		{
			output = true;
			accessByteData(queueTile, mapWidth).setDir(queueTile.dir, true);
			if (queueTile.dir == windDirection)
				queue.emplace(queueTile, tether - (1.0f - windStrength));
			else
				queue.emplace(queueTile, tether - 1);
		}
	}
	//If queue is not empty run algorithm again
	return output;
}

Ray2D PathFinding::nextTile(const Ray2D & currentTile, int mapWidth, int maxSize) const
{
	int x = currentTile.x;
	int y = currentTile.y;
	Ray2D nextAddress = currentTile;

	if (x & 1)//odd
	{
		switch (currentTile.dir)
		{
		case eNorth:
			nextAddress.x = x;
			nextAddress.y = y - 1;
			break;
		case eNorthEast:
			nextAddress.x = x + 1;
			nextAddress.y = y - 1;
			break;
		case eSouthEast:
			nextAddress.x = x + 1;
			nextAddress.y = y;
			break;
		case eSouth:
			nextAddress.x = x;
			nextAddress.y = y + 1;
			break;
		case eSouthWest:
			nextAddress.x = x - 1;
			nextAddress.y = y;
			break;
		case eNorthWest:
			nextAddress.x = x - 1;
			nextAddress.y = y - 1;
			break;
		}
	}
	else//even
	{
		switch (currentTile.dir)
		{
		case eNorth:
			nextAddress.x = x;
			nextAddress.y = y - 1;
			break;
		case eNorthEast:
			nextAddress.x = x + 1;
			nextAddress.y = y;
			break;
		case eSouthEast:
			nextAddress.x = x + 1;
			nextAddress.y = y + 1;
			break;
		case eSouth:
			nextAddress.x = x;
			nextAddress.y = y + 1;
			break;
		case eSouthWest:
			nextAddress.x = x - 1;
			nextAddress.y = y + 1;
			break;
		case eNorthWest:
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

Ray2D PathFinding::turnLeft(const Ray2D & currentTile) const
{
	Ray2D nextTile = currentTile;
	switch (currentTile.dir)
	{
	case eNorth:
		nextTile.dir = eNorthWest;
		break;
	case eNorthEast:
		nextTile.dir = eNorth;
		break;
	case eSouthEast:
		nextTile.dir = eNorthEast;
		break;
	case eSouth:
		nextTile.dir = eSouthEast;
		break;
	case eSouthWest:
		nextTile.dir = eSouth;
		break;
	case eNorthWest:
		nextTile.dir = eSouthWest;
		break;
	}

	return nextTile;
}

Ray2D PathFinding::turnRight(const Ray2D & currentTile) const
{
	Ray2D nextTile = currentTile;
	switch (currentTile.dir)
	{
	case eNorth:
		nextTile.dir = eNorthEast;
		break;
	case eNorthEast:
		nextTile.dir = eSouthEast;
		break;
	case eSouthEast:
		nextTile.dir = eSouth;
		break;
	case eSouth:
		nextTile.dir = eSouthWest;
		break;
	case eSouthWest:
		nextTile.dir = eNorthWest;
		break;
	case eNorthWest:
		nextTile.dir = eNorth;
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
		if ((map.getData()[i].m_type == eSea || map.getData()[i].m_type == eOcean) && !map.getData()[i].isShipOnTile())
		{
			tileTraversable = true;
		}

		m_tileData[i].m_traversable = tileTraversable;
		m_tileData[i].resetParent();
	}
}