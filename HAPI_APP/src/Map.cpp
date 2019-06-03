#include "Map.h"
#include <memory>
#include <math.h>
#include <algorithm>
#include "Utilities/Utilities.h"
#include "Textures.h"
#include "GameEventMessenger.h"
#include "Utilities/MapParser.h"
#include <algorithm>

constexpr int FRAME_HEIGHT{ 28 };
constexpr int FRAME_WIDTH{ 32 };
constexpr float FRAME_CENTRE_X{ 15.5 };
constexpr float FRAME_CENTRE_Y{ 32.5 };
constexpr float WIND_STRENGTH{ 0.3 };

//SpawnPosition
Map::SpawnPosition::SpawnPosition(sf::Vector2i spawnPosition)
	: position(spawnPosition),
	inUse(false)
{}

void Map::drawMap(sf::RenderWindow& window, eLightIntensity lightIntensity)
{
	//TODO: Whats the texture dimension
	sf::Vector2i textureDimensions = sf::Vector2i(FRAME_WIDTH, FRAME_HEIGHT);

	int access{ 0 };
	for (int y = 0; y < m_mapDimensions.y; y++)
	{
		const float yPosEven = (float)(0.5 + y) * textureDimensions.y;
		const float yPosOdd = (float)y * textureDimensions.y;

		for (int x = 1; x < m_mapDimensions.x; x += 2)
		{
			const float xPos = (float)x * textureDimensions.x * 3 / 4;
			int fin = access + x;
			switch (lightIntensity)
			{
			case eLightIntensity::eMaximum:
				m_data[fin].m_daySprite.setPosition(
					(xPos - m_drawOffset.x)*m_drawScale,
					(yPosOdd - m_drawOffset.y)*m_drawScale);

				m_data[fin].m_daySprite.setScale(m_drawScale, m_drawScale);
				window.draw(m_data[fin].m_daySprite);
				break;

			case eLightIntensity::eMinimum:
				m_data[fin].m_nightSprite.setPosition(
					(xPos - m_drawOffset.x)*m_drawScale,
					(yPosOdd - m_drawOffset.y)*m_drawScale);

				m_data[fin].m_nightSprite.setScale(m_drawScale, m_drawScale);
				window.draw(m_data[fin].m_nightSprite);
				break;
			}
			//Is Odd
		}
		for (int x = 0; x < m_mapDimensions.x; x += 2)
		{
			const float xPos = (float)x * textureDimensions.x * 3 / 4;
			//Is even
			switch (lightIntensity)
			{
			case eLightIntensity::eMaximum:
				m_data[access + x].m_daySprite.setPosition(
					(xPos - m_drawOffset.x)*m_drawScale,
					(yPosEven - m_drawOffset.y)*m_drawScale);

				m_data[access + x].m_daySprite.setScale(m_drawScale, m_drawScale);
				window.draw(m_data[access + x].m_daySprite);
				break;
			case eLightIntensity::eMinimum:
				m_data[access + x].m_nightSprite.setPosition(
					(xPos - m_drawOffset.x)*m_drawScale,
					(yPosEven - m_drawOffset.y)*m_drawScale);

				m_data[access + x].m_nightSprite.setScale(m_drawScale, m_drawScale);
				window.draw(m_data[access + x].m_nightSprite);
				break;
			}
		}
		access += m_mapDimensions.x;
	}
}

Map::~Map()
{
	GameEventMessenger::getInstance().unsubscribe("Map", GameEvent::eResetBattle);
}

sf::Vector2i Map::offsetToCube(sf::Vector2i offset) const
{
	int cubeX = offset.x;
	int cubeY = - offset.x - (offset.y - (offset.x + (offset.x & 1)) / 2);
	int cubeZ = -cubeX - cubeY;
	return sf::Vector2i(cubeX, cubeY);
}

sf::Vector2i Map::cubeToOffset(sf::Vector2i cube) const
{
	int offsetX = cube.x;
	int offsetY = -cube.x - cube.y + (cube.x + (cube.x & 1)) / 2;
	return sf::Vector2i(offsetX, offsetY);
}

int Map::cubeDistance(sf::Vector2i a, sf::Vector2i b) const
{
	const int x = abs(a.x - b.x);
	const int y = abs(a.y - b.y);
	const int z = abs(a.x + a.y - b.x - b.y);
	return std::max(x, std::max(y, z));
}

bool Map::inCone(sf::Vector2i orgHex, sf::Vector2i testHex, eDirection dir) const
{
	const sf::Vector2i diff(testHex.x - orgHex.x, testHex.y - orgHex.y);
	const int zDiff = -diff.x - diff.y;
	if (dir == eNorth || dir == eSouth)//Axis x = 0
	{
		//Deadzones y pos and z neg or y neg and z pos
		if ((diff.y > 0 && zDiff < 0) || (diff.y < 0 && zDiff > 0))
			return false;
	}
	else if (dir == eNorthEast || dir == eSouthWest)//Axis y = 0
	{
		//Deadzones x pos and z neg or x neg and z pos
		if ((diff.x > 0 && zDiff < 0) || (diff.x < 0 && zDiff > 0))
			return false;
	}
	else if (dir == eNorthWest || dir == eSouthEast)//Axis z = 0
	{
		//Deadzones x pos and y neg or x neg and y pos
		if ((diff.x > 0 && diff.y < 0) || (diff.x < 0 && diff.y > 0))
			return false;
	}
	return true;
}

float Map::tileDistanceMag(sf::Vector2i tileCoord, sf::Vector2i mouseClick) const
{
	sf::Vector2f tileCentre = sf::Vector2f(getTileScreenPos(tileCoord).x, getTileScreenPos(tileCoord).y);
	tileCentre.x += FRAME_CENTRE_X * m_drawScale;
	tileCentre.y += FRAME_CENTRE_Y * m_drawScale;

	const float diffX = tileCentre.x - static_cast<float>(mouseClick.x);
	const float diffY = tileCentre.y - static_cast<float>(mouseClick.y);

	return (diffX * diffX) + (diffY * diffY);
}

Tile* Map::getTile(sf::Vector2i coordinate)
{
	//Bounds check
	if (coordinate.x < m_mapDimensions.x &&
		coordinate.y < m_mapDimensions.y &&
		coordinate.x >= 0 &&
		coordinate.y >= 0)
	{	 
		return &m_data[coordinate.x + coordinate.y * m_mapDimensions.x];
	}
	/*
	HAPI_Sprites.UserMessage(
		std::string("getTile request out of bounds: " + std::to_string(coordinate.x) +
			", " + std::to_string(coordinate.y) + " map dimensions are: " +
			std::to_string(m_mapDimensions.x) +", "+ std::to_string(m_mapDimensions.y)),
		"Map error");
	*/
	return nullptr;
}

std::vector<Tile*> Map::getAdjacentTiles(sf::Vector2i coord)
{
	const size_t allAdjacentTiles = 6;
	std::vector<Tile*> result;
	result.reserve(size_t(allAdjacentTiles));
	if (coord.x & 1)//Is an odd tile
	{
		result.push_back(getTile(sf::Vector2i(coord.x, coord.y - 1)));		//N
		result.push_back(getTile(sf::Vector2i(coord.x + 1, coord.y - 1)));	//NE
		result.push_back(getTile(sf::Vector2i(coord.x + 1, coord.y)));		//SE
		result.push_back(getTile(sf::Vector2i(coord.x, coord.y + 1)));		//S
		result.push_back(getTile(sf::Vector2i(coord.x - 1, coord.y)));		//SW
		result.push_back(getTile(sf::Vector2i(coord.x - 1, coord.y - 1)));	//NW
	}
	else//Is even
	{
		result.push_back(getTile(sf::Vector2i(coord.x, coord.y - 1)));		//N
		result.push_back(getTile(sf::Vector2i(coord.x + 1, coord.y)));		//NE
		result.push_back(getTile(sf::Vector2i(coord.x + 1, coord.y + 1)));	//SE
		result.push_back(getTile(sf::Vector2i(coord.x, coord.y + 1)));		//S
		result.push_back(getTile(sf::Vector2i(coord.x - 1, coord.y + 1)));	//SW
		result.push_back(getTile(sf::Vector2i(coord.x - 1, coord.y)));		//NW
	}
	return result;
}

std::vector<Tile*> Map::getTileRadius(sf::Vector2i coord, int range, bool avoidInvalid, bool includeSource)
{
	assert(range > 1);

	int reserveSize{ 0 };
	if (includeSource)
		reserveSize++;
	for (int i = 1; i <= range; i++)
	{
		reserveSize += 6 * i;
	}
	std::vector<Tile*> tileStore;
	tileStore.reserve((size_t)reserveSize);
	if ((includeSource && !avoidInvalid) || (includeSource && avoidInvalid && (getTile(coord)->m_type == eSea || getTile(coord)->m_type == eOcean)))
	{
		tileStore.push_back(getTile(coord));
	}
	
	sf::Vector2i cubeCoord(offsetToCube(coord));

	for (int y = std::max(0, coord.y - range);
		y < std::min(m_mapDimensions.y, coord.y + range + 1);
		y++)
	{
		for (int x = std::max(0, coord.x - range);
			x < std::min(m_mapDimensions.x, coord.x + range + 1);
			x++)
		{
			if (!(coord.x == x && coord.y == y))//If not the tile at the centre
			{
				if (cubeDistance(cubeCoord, offsetToCube(sf::Vector2i(x, y))) <= range)
				{
					Tile* pushBackTile = getTile(sf::Vector2i(x, y));
					if (!pushBackTile)
						continue;
					if (avoidInvalid)
					{
						if (!(pushBackTile->m_type != eTileType::eSea && pushBackTile->m_type != eTileType::eOcean))
						{
							tileStore.push_back(getTile(sf::Vector2i(x, y)));
						}
					}
					else
					{
						tileStore.push_back(getTile(sf::Vector2i(x, y)));
					}
				}
			}
		}
	}
	return tileStore;
}

std::vector<Tile*> Map::getTileCone(sf::Vector2i coord, int range, eDirection direction, bool avoidInvalid)
{
	assert(range > 1);

	int reserveSize{ 0 };
	for (int i = 2; i < range + 2; i++)
	{
		reserveSize += 2 * i;
	}
	std::vector<Tile*> tileStore;
	tileStore.reserve((size_t)reserveSize);

	const sf::Vector2i cubeCoord(offsetToCube(coord));

	for (int y = std::max(0, coord.y - range - 1);
		y < std::min(m_mapDimensions.y, coord.y + range + 2);
		y++)
	{
		for (int x = std::max(0, coord.x - range - 1);
			x < std::min(m_mapDimensions.x, coord.x + range + 2);
			x++)
		{
			if (!(coord.x == x && coord.y == y))//If not the tile at the centre
			{
				sf::Vector2i tempCube(offsetToCube(sf::Vector2i(x, y)));
				if (cubeDistance(cubeCoord, tempCube) <= range)
				{
					if (inCone(cubeCoord, tempCube, direction))
					{
						Tile* pushBackTile = getTile(sf::Vector2i(x, y));
						if (!pushBackTile)
							continue;
						if (avoidInvalid)
						{
							if (!(pushBackTile->m_type != eTileType::eSea && pushBackTile->m_type != eTileType::eOcean))
							{
								tileStore.push_back(getTile(sf::Vector2i(x, y)));
							}
						}
						else
						{
							tileStore.push_back(getTile(sf::Vector2i(x, y)));
						}
					}
				}
			}
		}
	}
	return tileStore;
}

std::vector<const Tile*> Map::cGetTileCone(sf::Vector2i coord, int range, eDirection direction, bool avoidInvalid) const
{
	assert(range > 1);

	int reserveSize{ 0 };
	for (int i = 2; i < range + 2; i++)
	{
		reserveSize += 2 * i;
	}
	std::vector<const Tile*> tileStore;
	tileStore.reserve((size_t)reserveSize);

	const sf::Vector2i cubeCoord(offsetToCube(coord));

	for (int y = std::max(0, coord.y - range - 1);
		y < std::min(m_mapDimensions.y, coord.y + range + 2);
		y++)
	{
		for (int x = std::max(0, coord.x - range - 1);
			x < std::min(m_mapDimensions.x, coord.x + range + 2);
			x++)
		{
			if (!(coord.x == x && coord.y == y))//If not the tile at the centre
			{
				sf::Vector2i tempCube(offsetToCube(sf::Vector2i(x, y)));
				if (cubeDistance(cubeCoord, tempCube) <= range)
				{
					if (inCone(cubeCoord, tempCube, direction))
					{
						const Tile* pushBackTile = getTile(sf::Vector2i(x, y));
						if (!pushBackTile)
							continue;
						if (avoidInvalid)
						{
							if (!(pushBackTile->m_type != eTileType::eSea && pushBackTile->m_type != eTileType::eOcean))
							{
								tileStore.push_back(getTile(sf::Vector2i(x, y)));
							}
						}
						else
						{
							tileStore.push_back(getTile(sf::Vector2i(x, y)));
						}
					}
				}
			}
		}
	}
	return tileStore;
}

bool Map::updateShipOnTile(ShipOnTile ship, sf::Vector2i currentPosition, sf::Vector2i newPosition)
{
	Tile* currentTile = getTile(currentPosition);
	assert(currentTile);
	Tile* newTile = getTile(newPosition);
	assert(newTile);

	if (newTile->m_shipOnTile.isValid() || !currentTile->m_shipOnTile.isValid())
	{
		return false;
	}

	newTile->m_shipOnTile = currentTile->m_shipOnTile;
	currentTile->m_shipOnTile.reset();
	return true;
}

void Map::setShipOnTile(ShipOnTile ship, sf::Vector2i shipPosition)
{
	Tile* tile = getTile(shipPosition);	
	assert(tile);
	if (!tile->m_shipOnTile.isValid())
	{
		tile->m_shipOnTile = ship;
	}
}

sf::Vector2i Map::getTileScreenPos(sf::Vector2i coord) const
{
	sf::Vector2i textureDimensions = sf::Vector2i(FRAME_WIDTH, FRAME_HEIGHT);

	const float xPos = static_cast<float>(coord.x * textureDimensions.x) * 3 / 4;
	const float yPos = static_cast<float>((((1 + coord.x) % 2) + 2 * coord.y)
		* textureDimensions.y) / 2;

	return sf::Vector2i(
		(xPos - m_drawOffset.x)* m_drawScale,
		(yPos - m_drawOffset.y)* m_drawScale);
}

sf::Vector2i Map::getMouseClickCoord(sf::Vector2i mouseCoord) const
{
	sf::Vector2i textureDimensions = sf::Vector2i(FRAME_WIDTH, FRAME_HEIGHT);
	
	const float translatedX = (static_cast<float>(mouseCoord.x - FRAME_CENTRE_X) / m_drawScale) + static_cast<float>(m_drawOffset.x);
	const float translatedY = (static_cast<float>(mouseCoord.y - FRAME_CENTRE_Y) / m_drawScale) + static_cast<float>(m_drawOffset.y);
	const int predictedTileX = static_cast<const int>(translatedX * 4 / (3 * textureDimensions.x));
	const int predictedTileY = static_cast<const int>(translatedY / textureDimensions.y);
	
	float distance{ 10000000 };//An arbitrary big number
	sf::Vector2i closestTile(predictedTileX, predictedTileY);
	//Iterate through the 9 tiles around the guess to find the actual closest tile to the click
	for (int y = predictedTileY - 1; y <= predictedTileY + 1; y++)
	{
		for (int x = predictedTileX - 1; x <= predictedTileX + 1; x++)
		{
			const float tempDist = tileDistanceMag(sf::Vector2i(x, y), mouseCoord);
			if (tempDist < distance)
			{
				distance = tempDist;
				closestTile = sf::Vector2i(x, y);
			}
		}
	}
	return closestTile;
}

void Map::loadmap(const std::string & mapName)
{
	assert(!mapName.empty());
	MapDetails mapDetails = MapParser::parseMapDetails(mapName);
	m_mapDimensions = mapDetails.mapDimensions;
	m_data.reserve(m_mapDimensions.x * m_mapDimensions.y);
	for (auto spawnPosition : mapDetails.m_spawnPositions)
	{
		m_spawnPositions.push_back(spawnPosition);
	}

	for (int y = 0; y < m_mapDimensions.y; y++)
	{
		for (int x = 0; x < m_mapDimensions.x; x++)
		{
			const int tileID = mapDetails.tileData[y][x];
			assert(tileID != -1);
			m_data.emplace_back(static_cast<eTileType>(tileID),
				Textures::m_hexTiles, Textures::m_nightHexTiles, sf::Vector2i(x, y));

			//if (!m_data[x + y * m_mapDimensions.x].m_daySprite)
			//{
			//	HAPI_Sprites.UserMessage("Could not load tile spritesheet", "Error");
			//	return;
			//}
			/*m_data[x + y * m_mapDimensions.x].m_daySprite->SetFrameNumber(tileID);
			m_data[x + y * m_mapDimensions.x].m_nightSprite->SetFrameNumber(tileID);*/
		}
	}
}

Map::Map() :
	m_mapDimensions(0, 0),
	m_data(),
	m_drawOffset(sf::Vector2i(10, 60)),
	m_windDirection(eNorth),
	m_windStrength(WIND_STRENGTH),
	m_drawScale(2)
{
	GameEventMessenger::getInstance().subscribe(std::bind(&Map::onReset, this), "Map", GameEvent::eResetBattle);
}

void Map::onReset()
{
	m_data.clear();
	m_mapDimensions = sf::Vector2i(0, 0);
	m_drawOffset = sf::Vector2i(10, 60);
	m_drawScale = 2;

	m_windDirection = eNorth;
	m_windStrength = 0.3f;
	m_spawnPositions.clear();
}

const Tile * Map::getTile(sf::Vector2i coordinate) const
{
	//Bounds check
	if (coordinate.x < m_mapDimensions.x &&
		coordinate.y < m_mapDimensions.y &&
		coordinate.x >= 0 &&
		coordinate.y >= 0)
	{
		return &m_data[coordinate.x + coordinate.y * m_mapDimensions.x];
	}
	/*
	HAPI_Sprites.UserMessage(
		std::string("getTile request out of bounds: " + std::to_string(coordinate.x) +
			", " + std::to_string(coordinate.y) + " map dimensions are: " +
			std::to_string(m_mapDimensions.x) +", "+ std::to_string(m_mapDimensions.y)),
		"Map error");
	*/
	return nullptr;
}

Tile * Map::getTile(posi coordinate)
{
	//Bounds check
	if (coordinate.x < m_mapDimensions.x &&
		coordinate.y < m_mapDimensions.y &&
		coordinate.x >= 0 &&
		coordinate.y >= 0)
	{
		return &m_data[coordinate.x + coordinate.y * m_mapDimensions.x];
	}
	return nullptr;
}

const Tile * Map::getTile(posi coordinate) const
{
//Bounds check
if (coordinate.x < m_mapDimensions.x &&
	coordinate.y < m_mapDimensions.y &&
	coordinate.x >= 0 &&
	coordinate.y >= 0)
{
	return &m_data[coordinate.x + coordinate.y * m_mapDimensions.x];
}
return nullptr;
}

std::vector<const Tile*> Map::cGetAdjacentTiles(sf::Vector2i coord) const
{
	const size_t allAdjacentTiles = 6;
	std::vector<const Tile*> result;
	result.reserve(size_t(allAdjacentTiles));
	if (coord.x & 1)//Is an odd tile
	{
		result.push_back(getTile(sf::Vector2i(coord.x, coord.y - 1)));		//N
		result.push_back(getTile(sf::Vector2i(coord.x + 1, coord.y - 1)));	//NE
		result.push_back(getTile(sf::Vector2i(coord.x + 1, coord.y)));		//SE
		result.push_back(getTile(sf::Vector2i(coord.x, coord.y + 1)));		//S
		result.push_back(getTile(sf::Vector2i(coord.x - 1, coord.y)));		//SW
		result.push_back(getTile(sf::Vector2i(coord.x - 1, coord.y - 1)));	//NW
	}
	else//Is even
	{
		result.push_back(getTile(sf::Vector2i(coord.x, coord.y - 1)));		//N
		result.push_back(getTile(sf::Vector2i(coord.x + 1, coord.y)));		//NE
		result.push_back(getTile(sf::Vector2i(coord.x + 1, coord.y + 1)));	//SE
		result.push_back(getTile(sf::Vector2i(coord.x, coord.y + 1)));		//S
		result.push_back(getTile(sf::Vector2i(coord.x - 1, coord.y + 1)));	//SW
		result.push_back(getTile(sf::Vector2i(coord.x - 1, coord.y)));		//NW
	}
	return result;
}

std::vector<const Tile*> Map::cGetTileRadius(sf::Vector2i coord, int range, bool avoidInvalid, bool includeSource) const
{
	int reserveSize{ 0 };
	if (includeSource)
		reserveSize++;
	for (int i = 1; i <= range; i++)
	{
		reserveSize += 6 * i;
	}
	std::vector<const Tile*> tileStore;
	tileStore.reserve((size_t)reserveSize);
	if ((includeSource && !avoidInvalid) || (includeSource && avoidInvalid && (getTile(coord)->m_type == eSea || getTile(coord)->m_type == eOcean)))
	{
		tileStore.push_back(getTile(coord));
	}

	sf::Vector2i cubeCoord(offsetToCube(coord));

	for (int y = std::max(0, coord.y - range);
		y < std::min(m_mapDimensions.y, coord.y + range + 1);
		y++)
	{
		for (int x = std::max(0, coord.x - range);
			x < std::min(m_mapDimensions.x, coord.x + range + 1);
			x++)
		{
			if (!(coord.x == x && coord.y == y))//If not the tile at the centre
			{
				if (cubeDistance(cubeCoord, offsetToCube(sf::Vector2i(x, y))) <= range)
				{
					const Tile* pushBackTile = getTile(sf::Vector2i(x, y));
					if (!pushBackTile)
						continue;
					if (avoidInvalid)
					{
						if (!(pushBackTile->m_type != eTileType::eSea && pushBackTile->m_type != eTileType::eOcean))
						{
							tileStore.push_back(getTile(sf::Vector2i(x, y)));
						}
					}
					else
					{
						tileStore.push_back(getTile(sf::Vector2i(x, y)));
					}
				}
			}
		}
	}
	return tileStore;
}

std::vector<Tile*> Map::getTileLine(
	sf::Vector2i coord, int range, eDirection direction, bool avoidInvalid)
{
	std::vector<Tile*> tileStore;
	tileStore.reserve(range);
	Tile* pushBackTile{ getTile(coord) };
	for (int i = 0; i < range; i++)
	{
		if (!pushBackTile)
			continue;
		pushBackTile = getAdjacentTiles(pushBackTile->m_tileCoordinate)[direction];
		//If avoidInvalid stop the line if a mountain or Mesa is encountered
		if (avoidInvalid && pushBackTile && (pushBackTile->m_type == eMountain || pushBackTile->m_type == eMesa))
			break;
		//If avoidInvalid skip if the tile is an entity or not water
		if (avoidInvalid && pushBackTile && (pushBackTile->m_type != eSea && pushBackTile->m_type != eOcean))
			continue;
		tileStore.emplace_back(pushBackTile);
	}
	return tileStore;
}

std::vector<const Tile*> Map::cGetTileLine(
	sf::Vector2i coord, int range, eDirection direction, bool avoidInvalid)const
{
	std::vector<const Tile*> tileStore;
	tileStore.reserve(range);
	const Tile* pushBackTile{ getTile(coord) };
	for (int i = 0; i < range; i++)
	{
		if (!pushBackTile)
			continue;
		pushBackTile = cGetAdjacentTiles(pushBackTile->m_tileCoordinate)[direction]; 
		//If avoidInvalid stop the line if a mountain or Mesa is encountered
		if (avoidInvalid && pushBackTile && (pushBackTile->m_type == eMountain || pushBackTile->m_type == eMesa))
			break;
		//If avoidInvalid skip if the tile is not water
		if (avoidInvalid && pushBackTile && (pushBackTile->m_type != eSea && pushBackTile->m_type != eOcean))
			continue;
		tileStore.emplace_back(pushBackTile);
	}
	return tileStore;
}

std::vector<Tile*> Map::getTileRing(sf::Vector2i coord, int range)
{
	std::vector<Tile*> tileStore;
	tileStore.reserve(6 * range);

	sf::Vector2i cubeCoord(offsetToCube(coord));

	for (int y = std::max(0, coord.y - range);
		y < std::min(m_mapDimensions.y, coord.y + range + 1);
		y++)
	{
		for (int x = std::max(0, coord.x - range);
			x < std::min(m_mapDimensions.x, coord.x + range + 1);
			x++)
		{
			if (!(coord.x == x && coord.y == y))//If not the tile at the centre
			{
				if (cubeDistance(cubeCoord, offsetToCube(sf::Vector2i(x, y))) == range)
				{
					tileStore.push_back(getTile(sf::Vector2i(x, y)));
				}
			}
		}
	}
	return tileStore;
}

std::vector<const Tile*> Map::cGetTileRing(sf::Vector2i coord, int range) const
{
	std::vector<const Tile*> tileStore;
	tileStore.reserve(6 * range);

	sf::Vector2i cubeCoord(offsetToCube(coord));

	for (int y = std::max(0, coord.y - range);
		y < std::min(m_mapDimensions.y, coord.y + range + 1);
		y++)
	{
		for (int x = std::max(0, coord.x - range);
			x < std::min(m_mapDimensions.x, coord.x + range + 1);
			x++)
		{
			if (!(coord.x == x && coord.y == y))//If not the tile at the centre
			{
				if (cubeDistance(cubeCoord, offsetToCube(sf::Vector2i(x, y))) == range)
				{
					tileStore.push_back(getTile(sf::Vector2i(x, y)));
				}
			}
		}
	}
	return tileStore;
}

sf::Vector2i Map::getRandomSpawnPosition()
{
	//Make sure all spawn positions aren't in use
	assert(std::find_if(m_spawnPositions.cbegin(), m_spawnPositions.cend(),
		[](const auto& spawnPosition) { return spawnPosition.inUse == false; }) != m_spawnPositions.cend());

	sf::Vector2i spawnPosition;
	bool validSpawnPositionFound = false;
	while (!validSpawnPositionFound)
	{
		int randNumb = Utilities::getRandomNumber(0, static_cast<int>(m_spawnPositions.size()) - 1);
		if (!m_spawnPositions[randNumb].inUse)
		{
			m_spawnPositions[randNumb].inUse = true;
			spawnPosition = m_spawnPositions[randNumb].position;
			validSpawnPositionFound = true;
		}
	}
	return spawnPosition;
}