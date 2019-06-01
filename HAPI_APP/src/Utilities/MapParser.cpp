#include "MapParser.h"
#include "Base64.h"
#include "tinyxml.h"
#include "../Map.h"
#include <assert.h>
#include "Utilities.h"

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
std::vector<sf::Vector2i> parseSpawnPositions(const TiXmlElement & rootElement, sf::Vector2i tileSize);

MapDetails MapParser::parseMapDetails(const std::string& name)
{
	TiXmlDocument mapFile;
	bool mapLoaded = mapFile.LoadFile(Utilities::getDataDirectory() + name);
	assert(mapLoaded);

	const auto& rootElement = mapFile.RootElement();
	sf::Vector2i mapSize = parseMapSize(*rootElement);
	sf::Vector2i tileSize = parseTileSize(*rootElement);
	std::vector<std::vector<int>> tileData = parseTileData(*rootElement, mapSize);
	std::vector<sf::Vector2i> spawnPositions = parseSpawnPositions(*rootElement, tileSize);

	return MapDetails(mapSize, std::move(tileData), std::move(spawnPositions));
}

std::vector<std::vector<int>> decodeTileLayer(const TiXmlElement & tileLayerElement, sf::Vector2i mapSize)
{
	std::vector<std::vector<int>> tileData;
	tileData.reserve(mapSize.second);

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

	const std::vector<int> layerColumns(mapSize.first);
	for (int i = 0; i < mapSize.second; ++i)
	{
		tileData.push_back(layerColumns);
	}

	for (int rows = 0; rows < mapSize.second; ++rows)
	{
		for (int cols = 0; cols < mapSize.first; ++cols)
		{
			tileData[rows][cols] = *((int*)decodedIDs.data() + rows * mapSize.first + cols) - 1;
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
	rootElement.Attribute("width", &mapSize.first);
	rootElement.Attribute("height", &mapSize.second);
	assert(mapSize.first != 0 && mapSize.second != 0);
	return mapSize;
}

sf::Vector2i parseTileSize(const TiXmlElement & rootElement)
{
	sf::Vector2i tileSize(0, 0);
	rootElement.Attribute("tilewidth", &tileSize.first);
	rootElement.Attribute("tileheight", &tileSize.second);
	assert(tileSize.first != 0 && tileSize.second != 0);
	return tileSize;
}

std::vector<sf::Vector2i> parseSpawnPositions(const TiXmlElement & rootElement, sf::Vector2i tileSize)
{
	std::vector<sf::Vector2i> entityStartingPositions;
	for (const auto* entityElementRoot = rootElement.FirstChildElement(); entityElementRoot != nullptr; entityElementRoot = entityElementRoot->NextSiblingElement())
	{
		if (entityElementRoot->Value() != std::string("objectgroup") || entityElementRoot->Attribute("name") != std::string("SpawnPositionLayer"))
		{
			continue;
		}

		for (const auto* entityElement = entityElementRoot->FirstChildElement(); entityElement != nullptr; entityElement = entityElement->NextSiblingElement())
		{
			sf::Vector2i startingPosition;
			entityElement->Attribute("x", &startingPosition.first);
			entityElement->Attribute("y", &startingPosition.second);
			//startingPosition.second -= tileSize; //Tiled Hack
			startingPosition.first /= 24;
			startingPosition.second /= 28;
			entityStartingPositions.push_back(startingPosition);
		}
	}
	assert(!entityStartingPositions.empty());
	return entityStartingPositions;
}