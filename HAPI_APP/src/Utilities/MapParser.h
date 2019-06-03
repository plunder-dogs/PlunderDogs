#pragma once

#include <string>
#include <SFML/Graphics.hpp>
#include <vector>

struct MapDetails
{
	MapDetails(sf::Vector2i mapSize, std::vector<std::vector<int>>&& tileData, 
		std::vector<sf::Vector2i>&& spawnPositions);

	const sf::Vector2i mapDimensions;
	const std::vector<std::vector<int>> tileData;
	const std::vector<sf::Vector2i> m_spawnPositions;
};

namespace MapParser
{
	MapDetails parseMapDetails(const std::string& name);
}