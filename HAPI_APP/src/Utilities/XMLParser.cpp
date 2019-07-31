#include "XMLParser.h"
#include "Base64.h"
#include "tinyxml.h"
#include "../Map.h"
#include <assert.h>
#include "Utilities.h"
#include "../Texture.h"

MapDetails::MapDetails(sf::Vector2i mapSize, std::vector<std::vector<int>>&& tileData,
	std::vector<sf::Vector2i>&& spawnPositions)
	: mapDimensions(mapSize),
	tileData(std::move(tileData)),
	m_spawnPositions(std::move(spawnPositions))
{}

std::vector<std::vector<int>> parseTileData(const TiXmlElement& rootElement, const sf::Vector2i mapSize);
sf::Vector2i parseMapSize(const TiXmlElement& rootElement);
sf::Vector2i parseTileSize(const TiXmlElement& rootElement);
std::vector<std::vector<int>> decodeTileLayer(const TiXmlElement & tileLayerElement, sf::Vector2i mapSize);
std::vector<sf::Vector2i> parseFactionSpawnPositions(const TiXmlElement & rootElement, sf::Vector2i tileSize);

void XMLParser::loadTexture(std::unique_ptr<Texture>& texture, const std::string& directory, const std::string& fileName)
{
	assert(!texture);
	TiXmlDocument file;
	bool fileLoaded = file.LoadFile(directory + fileName);
	assert(fileLoaded);

	const auto& rootElement = file.RootElement();
	std::vector<FrameDetails> frameDetails;
	int i = 0; //Acts as the frame ID for each iteration
	for (const TiXmlElement* e = rootElement->FirstChildElement(); e != nullptr; e = e->NextSiblingElement())
	{
		int height = 0;
		e->Attribute("height", &height);
		int width = 0;
		e->Attribute("width", &width);
		int y = 0;
		e->Attribute("y", &y);
		int x = 0;
		e->Attribute("x", &x);
		int frameID = i;
		++i;
		
		frameDetails.emplace_back(height, width, y, x, frameID);	
	}

	std::string imagePath = rootElement->Attribute("imagePath");
	texture = std::make_unique<Texture>(directory + imagePath, std::move(frameDetails));
}

MapDetails XMLParser::parseMapDetails(const std::string& name)
{
	TiXmlDocument mapFile;
	bool mapLoaded = mapFile.LoadFile(LEVEL_DATA_DIRECTORY + name);
	assert(mapLoaded);

	const auto& rootElement = mapFile.RootElement();
	sf::Vector2i mapSize = parseMapSize(*rootElement);
	sf::Vector2i tileSize = parseTileSize(*rootElement);
	std::vector<std::vector<int>> tileData = parseTileData(*rootElement, mapSize);
	std::vector<sf::Vector2i> spawnPositions = parseFactionSpawnPositions(*rootElement, tileSize);

	return MapDetails(mapSize, std::move(tileData), std::move(spawnPositions));
}

std::vector<std::vector<int>> decodeTileLayer(const TiXmlElement & tileLayerElement, sf::Vector2i mapSize)
{
	std::vector<std::vector<int>> tileData;
	tileData.reserve(mapSize.y);

	std::string decodedIDs; //Base64 decoded information
	const TiXmlElement* dataNode = nullptr; //Store our node once we find it
	for (const TiXmlElement* e = tileLayerElement.FirstChildElement(); e != nullptr; e = e->NextSiblingElement())
	{
		if (e->Value() == std::string("data"))
		{
			dataNode = e;
		}
	}
	assert(dataNode);

	Base64 base64;
	const TiXmlText* text = dataNode->FirstChild()->ToText();
	const std::string t = text->Value();
	decodedIDs = base64.base64_decode(t);

	const std::vector<int> layerColumns(mapSize.x);
	for (int i = 0; i < mapSize.y; ++i)
	{
		tileData.push_back(layerColumns);
	}

	for (int rows = 0; rows < mapSize.y; ++rows)
	{
		for (int cols = 0; cols < mapSize.x; ++cols)
		{
			tileData[rows][cols] = *((int*)decodedIDs.data() + rows * mapSize.x + cols) - 1;
		}
	}

	return tileData;
}

std::vector<std::vector<int>> parseTileData(const TiXmlElement & rootElement, const sf::Vector2i mapSize)
{
	std::vector<std::vector<int>> tileData;
	for (const auto* tileLayerElement = rootElement.FirstChildElement();
		tileLayerElement != nullptr; tileLayerElement = tileLayerElement->NextSiblingElement())
	{
		if (tileLayerElement->Value() != std::string("layer"))
		{
			continue;
		}

		tileData = decodeTileLayer(*tileLayerElement, mapSize);
	}

	assert(!tileData.empty());
	return tileData;
}

sf::Vector2i parseMapSize(const TiXmlElement & rootElement)
{
	sf::Vector2i mapSize(0, 0);
	rootElement.Attribute("width", &mapSize.x);
	rootElement.Attribute("height", &mapSize.y);
	assert(mapSize.x != 0 && mapSize.y != 0);
	return mapSize;
}

sf::Vector2i parseTileSize(const TiXmlElement & rootElement)
{
	sf::Vector2i tileSize(0, 0);
	rootElement.Attribute("tilewidth", &tileSize.x);
	rootElement.Attribute("tileheight", &tileSize.y);
	assert(tileSize.x != 0 && tileSize.y != 0);
	return tileSize;
}

std::vector<sf::Vector2i> parseFactionSpawnPositions(const TiXmlElement & rootElement, sf::Vector2i tileSize)
{
	std::vector<sf::Vector2i> factionSpawnPositions;
	for (const auto* entityElementRoot = rootElement.FirstChildElement(); entityElementRoot != nullptr; entityElementRoot = entityElementRoot->NextSiblingElement())
	{
		if (entityElementRoot->Value() != std::string("objectgroup") || entityElementRoot->Attribute("name") != std::string("SpawnPositionLayer"))
		{
			continue;
		}

		for (const auto* entityElement = entityElementRoot->FirstChildElement(); entityElement != nullptr; entityElement = entityElement->NextSiblingElement())
		{
			sf::Vector2i spawnPosition;
			entityElement->Attribute("x", &spawnPosition.x);
			entityElement->Attribute("y", &spawnPosition.y);
			//startingPosition.y -= tileSize; //Tiled Hack
			spawnPosition.x /= 24;
			spawnPosition.y /= 28;
			factionSpawnPositions.push_back(spawnPosition);
		}
	}
	assert(!factionSpawnPositions.empty());
	return factionSpawnPositions;
}