#pragma once

#include <string>
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>

struct MapDetails
{
	MapDetails(sf::Vector2i mapSize, std::vector<std::vector<int>>&& tileData, 
		std::vector<sf::Vector2i>&& spawnPositions);

	const sf::Vector2i mapDimensions;
	const std::vector<std::vector<int>> tileData;
	const std::vector<sf::Vector2i> m_spawnPositions;
};

struct Texture;
namespace XMLParser
{
	std::unique_ptr<Texture> parseTexture(const std::string& directory, const std::string& fileName);
	MapDetails parseMapDetails(const std::string& mapName);
}