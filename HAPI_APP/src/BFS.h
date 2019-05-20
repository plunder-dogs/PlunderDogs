#pragma once
#include <vector>
#include <queue>
#include "Global.h"

struct Tile;
class Map;

namespace BFS
{
	//For finding the optimal path for a ship, returns an empty queue if it can't be reached
	std::queue<posi> findPath(const Map& map, posi startPos, posi endPos, float maxMovement = 10);
	//For finding the possible movement area of a ship
	std::vector<posi> findArea(const Map& map, posi startPos, float maxMovement = 10);
};

