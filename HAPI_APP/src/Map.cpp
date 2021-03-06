#include "Map.h"
#include <memory>
#include <math.h>
#include <algorithm>
#include "Utilities/Utilities.h"
#include "Resources.h"
#include "GameEventMessenger.h"
#include "Utilities/XMLParser.h"
#include <algorithm>
#include <iostream>
#include <random>

constexpr int FRAME_HEIGHT{ 28 };
constexpr int FRAME_WIDTH{ 32 };
constexpr float FRAME_CENTRE_X{ 15.5 };
constexpr float FRAME_CENTRE_Y{ 32.5 };
constexpr float WIND_STRENGTH{ 0.3 };
constexpr int MAX_ADJACENT_TILE_RANGE{ 10 };

void Map::renderMap(sf::RenderWindow& window)
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
			m_data[fin].m_sprite.setPosition(sf::Vector2i(
				(xPos - m_drawOffset.x)*m_drawScale,
				(yPosOdd - m_drawOffset.y)*m_drawScale ));

			m_data[fin].m_sprite.setScale({ m_drawScale, m_drawScale });
			m_data[fin].m_sprite.render(window);
			//Is Odd
		}
		for (int x = 0; x < m_mapDimensions.x; x += 2)
		{
			const float xPos = (float)x * textureDimensions.x * 3 / 4;
			//Is even
			m_data[access + x].m_sprite.setPosition(sf::Vector2i(
				(xPos - m_drawOffset.x)*m_drawScale,
				(yPosEven - m_drawOffset.y)*m_drawScale ));

			m_data[access + x].m_sprite.setScale({ m_drawScale, m_drawScale });
			m_data[access + x].m_sprite.render(window);
		}
		access += m_mapDimensions.x;
	}
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
	if (dir == eDirection::eNorth || dir == eDirection::eSouth)//Axis x = 0
	{
		//Deadzones y pos and z neg or y neg and z pos
		if ((diff.y > 0 && zDiff < 0) || (diff.y < 0 && zDiff > 0))
			return false;
	}
	else if (dir == eDirection::eNorthEast || dir == eDirection::eSouthWest)//Axis y = 0
	{
		//Deadzones x pos and z neg or x neg and z pos
		if ((diff.x > 0 && zDiff < 0) || (diff.x < 0 && zDiff > 0))
			return false;
	}
	else if (dir == eDirection::eNorthWest || dir == eDirection::eSouthEast)//Axis z = 0
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

	return nullptr;
}

void Map::getTileCone(std::vector<const Tile*>& tileArea, sf::Vector2i coord, int range, eDirection direction, bool avoidInvalid) const
{
	if (range < 1)
	{
		return;
	}

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
								tileArea.push_back(getTile(sf::Vector2i(x, y)));
							}
						}
						else
						{
							tileArea.push_back(getTile(sf::Vector2i(x, y)));
						}
					}
				}
			}
		}
	}
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
	currentTile->m_shipOnTile.clear();
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

bool Map::loadmap(const std::string & mapName)
{
	assert(!mapName.empty());
	assert(m_data.empty());
	
	//Load in map
	std::vector<std::vector<int>> tileData;
	if (!XMLParser::loadMap(mapName, m_mapDimensions, tileData, m_spawnPositions))
	{
		std::cerr << "Cannot load Map: " << mapName << "\n";
		return false;
	}

	//Shuffle random spawn positions
	auto rng = std::default_random_engine{};
	std::shuffle(m_spawnPositions.begin(), m_spawnPositions.end(), rng);

	//Load in Map
	m_data.reserve(m_mapDimensions.x * m_mapDimensions.y);
	for (int y = 0; y < m_mapDimensions.y; y++)
	{
		for (int x = 0; x < m_mapDimensions.x; x++)
		{
			const int tileID = tileData[y][x];
			assert(tileID != -1);

			m_data.emplace_back(Textures::getInstance().getTexture(HEX_TILES), sf::Vector2i(x, y), tileID);
		}
	}

	return true;
}

bool Map::isTileCollidable(const Tile & tile) const
{
	if ((tile.m_type == eTileType::eOcean || tile.m_type == eTileType::eSea) && !tile.isShipOnTile())
	{
		return false;
	}
	else
	{
		return true;
	}
}

Map::Map() :
	m_mapDimensions(0, 0),
	m_data(),
	m_drawOffset(sf::Vector2i(10, 60)),
	m_windDirection(eDirection::eNorth),
	m_windStrength(WIND_STRENGTH),
	m_drawScale(2)
{}

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

Tile * Map::getTile(Ray2D coordinate)
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

const Tile * Map::getTile(Ray2D coordinate) const
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

std::vector<const Tile*> Map::getAdjacentTiles(sf::Vector2i coord) const
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

void Map::getAdjacentTiles(std::vector<const Tile*>& tileArea, sf::Vector2i coord) const
{
	if (coord.x & 1)//Is an odd tile
	{
		tileArea.push_back(getTile(sf::Vector2i(coord.x, coord.y - 1)));		//N
		tileArea.push_back(getTile(sf::Vector2i(coord.x + 1, coord.y - 1)));	//NE
		tileArea.push_back(getTile(sf::Vector2i(coord.x + 1, coord.y)));		//SE
		tileArea.push_back(getTile(sf::Vector2i(coord.x, coord.y + 1)));		//S
		tileArea.push_back(getTile(sf::Vector2i(coord.x - 1, coord.y)));		//SW
		tileArea.push_back(getTile(sf::Vector2i(coord.x - 1, coord.y - 1)));	//NW
	}
	else//Is even
	{
		tileArea.push_back(getTile(sf::Vector2i(coord.x, coord.y - 1)));		//N
		tileArea.push_back(getTile(sf::Vector2i(coord.x + 1, coord.y)));		//NE
		tileArea.push_back(getTile(sf::Vector2i(coord.x + 1, coord.y + 1)));	//SE
		tileArea.push_back(getTile(sf::Vector2i(coord.x, coord.y + 1)));		//S
		tileArea.push_back(getTile(sf::Vector2i(coord.x - 1, coord.y + 1)));	//SW
		tileArea.push_back(getTile(sf::Vector2i(coord.x - 1, coord.y)));		//NW
	}
}

void Map::getNonCollidableAdjacentTiles(std::vector<const Tile*>& tileArea, sf::Vector2i coord) const
{
	if (coord.x & 1)//Is an odd tile
	{
		const Tile* tile = getTile(sf::Vector2i(coord.x, coord.y - 1));//N
		if (tile && !isTileCollidable(*tile))
		{
			tileArea.push_back(tile);
		}

		tile = getTile(sf::Vector2i(coord.x + 1, coord.y - 1)); //NE
		if (tile && !isTileCollidable(*tile))
		{
			tileArea.push_back(tile);
		}

		tile = getTile(sf::Vector2i(coord.x + 1, coord.y)); //SE
		if (tile && !isTileCollidable(*tile))
		{
			tileArea.push_back(tile);
		}

		tile = getTile(sf::Vector2i(coord.x, coord.y + 1)); //S
		if (tile && !isTileCollidable(*tile))
		{
			tileArea.push_back(tile);
		}

		tile = getTile(sf::Vector2i(coord.x - 1, coord.y)); //SW
		if (tile && !isTileCollidable(*tile))
		{
			tileArea.push_back(tile);
		}

		tile = getTile(sf::Vector2i(coord.x - 1, coord.y - 1)); //NW
		if (tile && !isTileCollidable(*tile))
		{
			tileArea.push_back(tile);
		}
	}
	else//Is even
	{
		const Tile* tile = getTile(sf::Vector2i(coord.x, coord.y - 1));//N
		if (tile && !isTileCollidable(*tile))
		{
			tileArea.push_back(tile);
		}

		tile = getTile(sf::Vector2i(coord.x + 1, coord.y));
		if (tile && !isTileCollidable(*tile))
		{
			tileArea.push_back(tile);
		}

		tile = getTile(sf::Vector2i(coord.x + 1, coord.y + 1));
		if (tile && !isTileCollidable(*tile))
		{
			tileArea.push_back(tile);
		}

		tile = getTile(sf::Vector2i(coord.x, coord.y + 1));
		if (tile && !isTileCollidable(*tile))
		{
			tileArea.push_back(tile);
		}

		tile = getTile(sf::Vector2i(coord.x - 1, coord.y + 1));
		if (tile && !isTileCollidable(*tile))
		{
			tileArea.push_back(tile);
		}

		tile = getTile(sf::Vector2i(coord.x - 1, coord.y));
		if (tile && !isTileCollidable(*tile))
		{
			tileArea.push_back(tile);
		}
	}
}

const Tile * Map::getNonCollidableAdjacentTile(const std::vector<const Tile*>& tileArea, sf::Vector2i coord) const
{
	for(int range = 1; range <= MAX_ADJACENT_TILE_RANGE; ++range)
	{ 
		//If odd
		if (coord.x & 1)
		{
			const Tile* adjacentTile = getTile(sf::Vector2i(coord.x, coord.y - range));//N
			if (adjacentTile &&
				!isTileCollidable(*adjacentTile) && !Utilities::isPositionFound(tileArea, adjacentTile->m_tileCoordinate))
			{
				return adjacentTile;
			}

			adjacentTile = getTile(sf::Vector2i(coord.x + range, coord.y - range));//NE
			if (adjacentTile &&
				!isTileCollidable(*adjacentTile) && !Utilities::isPositionFound(tileArea, adjacentTile->m_tileCoordinate))
			{
				return adjacentTile;
			}

			adjacentTile = getTile(sf::Vector2i(coord.x + range, coord.y));//SE
			if (adjacentTile &&
				!isTileCollidable(*adjacentTile) && !Utilities::isPositionFound(tileArea, adjacentTile->m_tileCoordinate))
			{
				return adjacentTile;
			}

			adjacentTile = getTile(sf::Vector2i(coord.x, coord.y + range));//S
			if (adjacentTile &&
				!isTileCollidable(*adjacentTile) && !Utilities::isPositionFound(tileArea, adjacentTile->m_tileCoordinate))
			{
				return adjacentTile;
			}

			adjacentTile = getTile(sf::Vector2i(coord.x - range, coord.y));//SW
			if (adjacentTile &&
				!isTileCollidable(*adjacentTile) && !Utilities::isPositionFound(tileArea, adjacentTile->m_tileCoordinate))
			{
				return adjacentTile;
			}

			adjacentTile = getTile(sf::Vector2i(coord.x - range, coord.y - range));//NW
			if (adjacentTile &&
				!isTileCollidable(*adjacentTile) && !Utilities::isPositionFound(tileArea, adjacentTile->m_tileCoordinate))
			{
				return adjacentTile;
			}
		}
		//If even
		else
		{
			const Tile* adjacentTile = getTile(sf::Vector2i(coord.x, coord.y - range));//N
			if (adjacentTile &&
				!isTileCollidable(*adjacentTile) && !Utilities::isPositionFound(tileArea, adjacentTile->m_tileCoordinate))
			{
				return adjacentTile;
			}

			adjacentTile = getTile(sf::Vector2i(coord.x + range, coord.y));//NE
			if (adjacentTile &&
				!isTileCollidable(*adjacentTile) && !Utilities::isPositionFound(tileArea, adjacentTile->m_tileCoordinate))
			{
				return adjacentTile;
			}

			adjacentTile = getTile(sf::Vector2i(coord.x + range, coord.y + range));//SE
			if (adjacentTile &&
				!isTileCollidable(*adjacentTile) && !Utilities::isPositionFound(tileArea, adjacentTile->m_tileCoordinate))
			{
				return adjacentTile;
			}

			adjacentTile = getTile(sf::Vector2i(coord.x, coord.y + range));//S
			if (adjacentTile &&
				!isTileCollidable(*adjacentTile) && !Utilities::isPositionFound(tileArea, adjacentTile->m_tileCoordinate))
			{
				return adjacentTile;
			}

			adjacentTile = getTile(sf::Vector2i(coord.x - range, coord.y + range));//SW
			if (adjacentTile &&
				!isTileCollidable(*adjacentTile) && !Utilities::isPositionFound(tileArea, adjacentTile->m_tileCoordinate))
			{
				return adjacentTile;
			}

			adjacentTile = getTile(sf::Vector2i(coord.x - range, coord.y));//NW
			if (adjacentTile &&
				!isTileCollidable(*adjacentTile) && !Utilities::isPositionFound(tileArea, adjacentTile->m_tileCoordinate))
			{
				return adjacentTile;
			}
		}
	}

	//No adjacent tile found
	return nullptr;
}

void Map::getTileRadius(std::vector<const Tile*>& tileArea, sf::Vector2i coord, int range, bool avoidInvalid, bool includeSource) const
{
	if (getTile(coord)->m_type != eTileType::eSea && getTile(coord)->m_type != eTileType::eOcean)
	{
		return;
	}

	if ((includeSource && !avoidInvalid) || (includeSource && avoidInvalid && 
		(getTile(coord)->m_type == eTileType::eSea || getTile(coord)->m_type == eTileType::eOcean)))
	{
		tileArea.push_back(getTile(coord));
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
							tileArea.push_back(getTile(sf::Vector2i(x, y)));
						}
					}
					else
					{
						tileArea.push_back(getTile(sf::Vector2i(x, y)));
					}
				}
			}
		}
	}
}

void Map::getTileLine(std::vector<const Tile*>& tileArea, sf::Vector2i coord, int range, eDirection direction, bool avoidInvalid) const
{
	const Tile* pushBackTile{ getTile(coord) };
	for (int i = 0; i < range; i++)
	{
		if (!pushBackTile)
			continue;
		pushBackTile = getAdjacentTiles(pushBackTile->m_tileCoordinate)[static_cast<int>(direction)];
		//If avoidInvalid stop the line if a mountain or Mesa is encountered
		if (avoidInvalid && pushBackTile && (pushBackTile->m_type == eTileType::eMountain || pushBackTile->m_type == eTileType::eMesa))
			break;
		//If avoidInvalid skip if the tile is not water
		if (avoidInvalid && pushBackTile && (pushBackTile->m_type != eTileType::eSea && pushBackTile->m_type != eTileType::eOcean))
			continue;
		tileArea.push_back(pushBackTile);
	}
}

void Map::getTileRing(std::vector<const Tile*>& tileArea, sf::Vector2i coord, int range) const
{
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
					tileArea.push_back(getTile(sf::Vector2i(x, y)));
				}
			}
		}
	}
}

sf::Vector2i Map::getSpawnPosition()
{
	assert(!m_spawnPositions.empty());
	sf::Vector2i spawnPosition = m_spawnPositions.back();
	m_spawnPositions.pop_back();
	return spawnPosition;
}