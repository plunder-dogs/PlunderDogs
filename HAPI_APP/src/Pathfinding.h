#pragma once

#include <deque>
#include "Global.h"

class Map;

namespace PathFinding
{
	std::deque<std::pair<eDirection, std::pair<int, int>>> getPathToTile(const Map &map, std::pair<int, int> src, std::pair<int, int> dest);
}