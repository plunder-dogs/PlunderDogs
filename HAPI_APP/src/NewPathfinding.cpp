#include "NewPathfinding.h"
#include "Map.h"
#include <vector>
#include <algorithm>

#define NO_TILE posi(-1, -1, eNorth)

struct tileData
{
	const bool isTraversable;
	const bool isOccupied;
	//The node that was first used to access the corresponding direction during the BFS
	//One for each direction in order
	posi parent[6];

	tileData(bool traversable, bool occupied) :
		isTraversable(traversable), isOccupied(occupied),
		parent{ NO_TILE, NO_TILE, NO_TILE, NO_TILE, NO_TILE, NO_TILE } {}
};

struct finderMap
{
	int width;
	std::vector<tileData> data;

	finderMap(const Map& map);
	tileData& access(posi tile) { return data[tile.x + tile.y * width]; }
};

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
	//Second bit
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

struct boolMap
{
	int width;
	std::vector<byteStore> data;

	boolMap(const Map& map);
	byteStore& access(posi tile) { return data[tile.x + tile.y * width]; }
};

posi nextTile(const posi& currentTile, const int mapWidth, const int maxSize);
posi turnLeft(const posi& currentTile);
posi turnRight(const posi& currentTile);

//This is the recursive algorithm that hunts for the assigned tile
posi pathExplorer(finderMap& exploreArea, std::queue<std::pair<posi, float>>& queue, posi destination, const eDirection windDirection, const float windStrength)
{
	//Dequeue a tile
	posi tile = queue.front().first;
	float tether = queue.front().second;
	queue.pop();
	//Check if there's enough movement to do check other tiles
	if (tether >= 0.0f)
	{
		//Check if this is the destination
		if (tile.pair() == destination.pair())
			return tile;
		//Check if any movements are unexplored and movable, if so set those to true and set their parent to this tile
		//Then enqueue left, right, and forward as appropriate:
		//Left
		posi queueTile = turnLeft(tile);
		if (exploreArea.access(queueTile).parent[queueTile.dir] == NO_TILE)
		{
			exploreArea.access(queueTile).parent[queueTile.dir] = tile;
			queue.emplace(queueTile, tether - 1);
		}
		//Right
		queueTile = turnRight(tile);
		if (exploreArea.access(queueTile).parent[queueTile.dir] == NO_TILE)
		{
			exploreArea.access(queueTile).parent[queueTile.dir] = tile;
			queue.emplace(queueTile, tether - 1);
		}
		//Forward
		queueTile =  nextTile(tile, exploreArea.width, exploreArea.data.size());
		if (queueTile != NO_TILE &&
			exploreArea.access(queueTile).parent[queueTile.dir] == NO_TILE &&
			exploreArea.access(queueTile).isTraversable &&
			!exploreArea.access(queueTile).isOccupied)
		{
			exploreArea.access(queueTile).parent[queueTile.dir] = tile;
			if (queueTile.dir == windDirection)
				queue.emplace(queueTile, tether - (1.0f - windStrength));
			else
				queue.emplace(queueTile, tether - 1);
		}
	}
	//If queue is not empty run algorithm again
	posi final{ NO_TILE };
	if (!queue.empty())
		final = pathExplorer(exploreArea, queue, destination, windDirection, windStrength);
	return final;
}

std::queue<posi> BFS::findPath(const Map& map, posi startPos, posi endPos, float maxMovement)
{
	//No bullshit
	if (!map.getTile(startPos) || !map.getTile(endPos) ||
		map.getTile(endPos)->m_entityOnTile ||
		(map.getTile(endPos)->m_type != eSea && map.getTile(endPos)->m_type != eOcean))
		return std::queue<posi>();
	//Initialise variables
	finderMap exploreArea(map);
	std::queue<std::pair<posi, float>> exploreQueue;
	//Add first element and set it to explored
	exploreQueue.emplace(startPos, maxMovement);
	exploreArea.access(startPos).parent[startPos.dir] = startPos;//Yes, it's set = itself as it's the root note
	//Start recursion
	posi trace = pathExplorer(exploreArea, exploreQueue, endPos, map.getWindDirection(), map.getWindStrength());
	if (trace == NO_TILE)
		return std::queue<posi>();
	//Trace path back from destination via parents
	std::vector<posi> pathToTile;
	pathToTile.reserve(25);
	while (trace != startPos)
	{
		pathToTile.emplace_back(trace);
		trace = exploreArea.access(trace).parent[trace.dir];
	}
	//Invert for convenience and fetch tile* for each address
	std::queue<posi> finalPath;
	for (int i = pathToTile.size() - 1; i >= 0; i--)
	{
		finalPath.emplace(pathToTile[i]);
	}
	return finalPath;
}

//This is the recursive algorithm that hunts for the assigned tile
int areaExplorer(boolMap& exploreArea, std::queue<std::pair<posi, float>>& queue, const eDirection windDirection, const float windStrength, int count = 1)
{
	//Dequeue a tile
	posi tile = queue.front().first;
	float tether = queue.front().second;
	queue.pop();
	//Check if there's enough movement to do check other tiles
	if (tether >= 0.0f)
	{
		//Set tile to show in result
		exploreArea.access(tile).setEncountered(true);
		//Check if any movements are unexplored and movable, if so set those to true and set their parent to this tile
		//Then enqueue left, right, and forward as appropriate:
		//Left
		posi queueTile = turnLeft(tile);
		if (!exploreArea.access(queueTile).getDir(queueTile.dir))
		{
			exploreArea.access(queueTile).setDir(queueTile.dir, true);
			queue.emplace(queueTile, tether - 1);
		}
		//Right
		queueTile = turnRight(tile);
		if (!exploreArea.access(queueTile).getDir(queueTile.dir))
		{
			exploreArea.access(queueTile).setDir(queueTile.dir, true);
			queue.emplace(queueTile, tether - 1);
		}
		//Forward
		queueTile = nextTile(tile, exploreArea.width, exploreArea.data.size());
		if (queueTile != NO_TILE &&
			!exploreArea.access(queueTile).getDir(queueTile.dir) &&
			exploreArea.access(queueTile).traversable())
		{
			count++;
			exploreArea.access(queueTile).setDir(queueTile.dir, true);
			if (queueTile.dir == windDirection)
				queue.emplace(queueTile, tether - (1.0f - windStrength));
			else
				queue.emplace(queueTile, tether - 1);
		}
	}
	//If queue is not empty run algorithm again
	if (!queue.empty())
		count = areaExplorer(exploreArea, queue, windDirection, windStrength, count);
	return count;
}

std::vector<posi> BFS::findArea(const Map & map, posi startPos, float maxMovement)
{
	//No bullshit
	if (!map.getTile(startPos))
		return std::vector<posi>();
	//Initialise variables
	boolMap exploreArea(map);
	std::queue<std::pair<posi, float>> exploreQueue;
	//Add first element and set it to explored
	exploreQueue.emplace(startPos, maxMovement);
	exploreArea.access(startPos).setDir(startPos.dir, true);//Yes, it's set = itself as it's the root note
	//Start recursion
	int areaSize = areaExplorer(exploreArea, exploreQueue, map.getWindDirection(), map.getWindStrength());
	//Iterate through exploreArea and pushback to the return vector
	std::vector<posi> allowedArea;
	allowedArea.reserve(areaSize);
	for (int i = 0; i < exploreArea.data.size(); i++)
	{
		if (exploreArea.data[i].encountered())
			allowedArea.emplace_back(i % exploreArea.width, i / exploreArea.width);
	}
	return allowedArea;
}

finderMap::finderMap(const Map& map) : width(map.getDimensions().first)
{
	data.reserve(map.getDimensions().first * map.getDimensions().second);
	for (const Tile& it : map.getData())
	{
		bool traversable = (it.m_type == eSea || it.m_type == eOcean);
		bool occupied = static_cast<bool>(it.m_entityOnTile);
		data.emplace_back(traversable, occupied);
	}
}

boolMap::boolMap(const Map& map) : width(map.getDimensions().first)
{
	data.reserve(map.getDimensions().first * map.getDimensions().second);
	for (const Tile& it : map.getData())
	{
		bool traversable = ((it.m_type == eSea || it.m_type == eOcean) && !static_cast<bool>(it.m_entityOnTile));
		data.emplace_back(byteStore(traversable));
	}
}

posi nextTile(const posi& currentTile, const int mapWidth, const int maxSize)
{
	int x = currentTile.x;
	int y = currentTile.y;
	posi nextAddress = currentTile;
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
		nextAddress = NO_TILE;
	return nextAddress;
}

posi turnLeft(const posi& currentTile)
{
	posi nextTile = currentTile;
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

posi turnRight(const posi& currentTile)
{
	posi nextTile = currentTile;
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