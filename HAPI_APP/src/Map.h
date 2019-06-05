#pragma once
#include <utility>
#include <vector>
#include <string>
#include <SFML/Graphics.hpp>
#include "global.h"

struct ShipOnTile
{
	ShipOnTile()
		: factionName(),
		shipID(INVALID_SHIP_ID)
	{}
	ShipOnTile(FactionName factionName, int shipID)
		: factionName(factionName),
		shipID(shipID)
	{}

	bool isValid() const
	{
		return (shipID != INVALID_SHIP_ID);
	}

	void clear()
	{
		factionName = FactionName::Invalid;
		shipID = INVALID_SHIP_ID;
	}

	FactionName factionName;
	int shipID;
};

class Ship;
struct Tile
{
	const enum eTileType m_type;
	ShipOnTile m_shipOnTile;
	FactionName m_shipOnTileFaction;
	sf::Sprite m_daySprite;
	const sf::Vector2i m_tileCoordinate;

	Tile(eTileType type, std::unique_ptr<sf::Texture>& dayTexture,
						sf::Vector2i coord, sf::IntRect textureRect) :
		m_type(type),
		m_shipOnTile(),
		m_daySprite(*dayTexture),
		m_tileCoordinate(coord)
	{
		m_daySprite.setTextureRect(textureRect);
	}
};

class Map
{
	struct SpawnPosition
	{
		SpawnPosition(sf::Vector2i spawnPosition);

		sf::Vector2i position;
		bool inUse;
	};

private:
	sf::Vector2i m_mapDimensions;
	float m_windStrength;
	eDirection m_windDirection;

	float m_drawScale;
	sf::Vector2i m_drawOffset;
	std::vector<Tile> m_data;
	std::vector<SpawnPosition> m_spawnPositions;

	sf::Vector2i offsetToCube(sf::Vector2i offset) const;
	sf::Vector2i cubeToOffset(sf::Vector2i cube) const;
	int cubeDistance(sf::Vector2i a, sf::Vector2i b) const;
	bool inCone(sf::Vector2i orgHex, sf::Vector2i testHex, eDirection dir) const;
	//Finds the euclidean distance from a point to a tile's centre, used by getMouseClickCoord
	float tileDistanceMag(sf::Vector2i tileCoord, sf::Vector2i mouseClick) const;
	void onReset();
public:
	//Returns a pointer to a given tile, returns nullptr if there is no tile there
	Tile* getTile(sf::Vector2i coordinate);
	const Tile* getTile(sf::Vector2i coordinate) const;
	Tile* getTile(posi coordinate);
	const Tile* getTile(posi coordinate) const;
	//An n = 1 version of getTileRadius for use in pathfinding, 
	//returns nullptr for each tile out of bounds
	std::vector<Tile*> getAdjacentTiles(sf::Vector2i coord);
	std::vector<const Tile*> cGetAdjacentTiles(sf::Vector2i coord) const;
	//Returns tiles in a radius around a given tile, skipping the tile itself
	std::vector<Tile*> getTileRadius(sf::Vector2i coord, int range, bool avoidInvalid = false, bool includeSource = false);
	std::vector<const Tile*> cGetTileRadius(sf::Vector2i coord, int range, bool avoidInvalid = false, bool includeSource = false) const;
	//Returns tiles in two cones emanating from a given tile, skipping the tile itself
	std::vector<Tile*> getTileCone(sf::Vector2i coord, int range, eDirection direction, bool avoidInvalid = false);
	std::vector<const Tile*> cGetTileCone(sf::Vector2i coord, int range, eDirection direction, bool avoidInvalid = false)const;
	//Returns tiles in a line from a given direction,
	//An element in the vector will be nullptr if it accesses an invalid tile
	std::vector<Tile*> getTileLine(sf::Vector2i coord, int range, eDirection direction, bool avoidInvalid = false);
	std::vector<const Tile*> cGetTileLine(sf::Vector2i coord, int range, eDirection direction, bool avoidInvalid = false)const;
	//Returns a ring of tiles at a certain radius from a specified tile
	//An element in the vector will be nullptr if it accesses an invalid tile
	std::vector<Tile*> getTileRing(sf::Vector2i coord, int range);
	std::vector<const Tile*> cGetTileRing(sf::Vector2i coord, int range)const;

	sf::Vector2i getRandomSpawnPosition();
	//For finding the location on the screen a given tile is being drawn
	sf::Vector2i getTileScreenPos(sf::Vector2i coord) const;

	//For finding the closest tile to the current mouse location, 
	//can give values that aren't valid tiles if you click off the map 
	//so check if getTile is null before using
	sf::Vector2i getMouseClickCoord(sf::Vector2i mouseCoord) const;

	//Moves an entitys position on the map, returns false if the position is already taken
	bool updateShipOnTile(ShipOnTile ship, sf::Vector2i currentPosition, sf::Vector2i newPosition);
	//Places a new entity on the map (no check for duplicates yet so try to avoid creating multiples)
	void setShipOnTile(ShipOnTile ship, sf::Vector2i shipPosition);

	void drawMap(sf::RenderWindow& window);
	sf::Vector2i getDrawOffset() const { return m_drawOffset; }
	void setDrawOffset(sf::Vector2i newOffset) { m_drawOffset = newOffset; }

	sf::Vector2i getDimensions() const { return m_mapDimensions; }

	float getDrawScale() const { return m_drawScale; }
	void setDrawScale(float scale) { if (scale > 0.0) m_drawScale = scale; }

	float getWindStrength() const { return m_windStrength; }
	void setWindStrength(float strength) { if (strength > 0.0) m_windStrength = strength; }

	eDirection getWindDirection() const { return m_windDirection; }
	void setWindDirection(eDirection direction) { m_windDirection = direction; }
	//TODO: Find out what this is
	//std::vector<sf::Vector2i> getSpawnPositions() const { return m_spawnPositions; }

	void loadmap(const std::string& mapName);

	//Only for pathfinding
	const std::vector<Tile>& getData()const { return m_data; }

	Map();
	Map(const Map&) = delete;
	Map& operator=(const Map&) = delete;
	Map(Map&&) = delete;
	Map&& operator=(Map&&) = delete;
	~Map();
};