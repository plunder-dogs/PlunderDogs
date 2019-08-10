#pragma once

#include <string>
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>

struct Texture;
namespace XMLParser
{
	std::unique_ptr<Texture> parseTexture(const std::string& directory, const std::string& fileName);

	bool loadMap(const std::string& mapName, sf::Vector2i& mapDimensions, 
		std::vector<std::vector<int>>& tileData, std::vector<sf::Vector2i>& spawnPositions);
}