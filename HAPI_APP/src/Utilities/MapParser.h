#pragma once

#include <string>
#include <vector>

struct MapDetails
{
	MapDetails(std::pair<int, int> mapSize, std::vector<std::vector<int>>&& tileData, 
		std::vector<std::pair<int, int>>&& spawnPositions);

	const std::pair<int, int> mapDimensions;
	const std::vector<std::vector<int>> tileData;
	const std::vector<std::pair<int, int>> m_spawnPositions;
};

namespace MapParser
{
	MapDetails parseMapDetails(const std::string& name);
}