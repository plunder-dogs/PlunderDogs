#pragma once

#include <string>
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>

struct FrameDetails;
class Texture;
namespace XMLParser
{
	bool loadTextureDetails(const std::string& fileName, std::string& imagePath, std::vector<FrameDetails>& frames);

	bool loadMap(const std::string& mapName, sf::Vector2i& mapDimensions, 
		std::vector<std::vector<int>>& tileData, std::vector<sf::Vector2i>& spawnPositions);
}