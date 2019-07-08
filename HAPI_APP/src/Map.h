#pragma once
#include <utility>
#include <vector>
#include <string>
#include <SFML/Graphics.hpp>
#include "Sprite.h"
#include "ShipOnTile.h"

struct Tile
{
	Tile(const Texture& dayTexture,
		sf::Vector2i coord, int tileID) :
		m_type(static_cast<eTileType>(tileID)),
		m_shipOnTile(),
		m_sprite(dayTexture),
		m_tileCoordinate(coord)
	{
		m_sprite.setFrameID(tileID);
	}

	bool isShipOnTile() const { return m_shipOnTile.isValid(); }

	const enum eTileType m_type;
	ShipOnTile m_shipOnTile;
	Sprite m_sprite;
	const sf::Vector2i m_tileCoordinate;
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
public:
	//Returns a pointer to a given tile, returns nullptr if there is no tile there
	Tile* getTile(sf::Vector2i coordinate);
	const Tile* getTile(sf::Vector2i coordinate) const;
	Tile* getTile(posi coordinate);
	const Tile* getTile(posi coordinate) const;

	//An n = 1 version of getTileRadius for use in pathfinding, 
	//returns nullptr for each tile out of bounds
	//std::vector<Tile*> getAdjacentTiles(sf::Vector2i coord);
	std::vector<const Tile*> getAdjacentTiles(sf::Vector2i coord) const;
	void getAdjacentTiles(std::vector<const Tile*>& tileArea, sf::Vector2i coord) const;
	void getNonCollidableAdjacentTiles(std::vector<const Tile*>& tileArea, sf::Vector2i coord) const;
	//Get adjacent tile from tile area
	const Tile* getNonCollidableAdjacentTile(const std::vector<const Tile*>& tileArea, int range) const;

	//Returns tiles in a radius around a given tile, skipping the tile itself
	std::vector<const Tile*> getTileRadius(sf::Vector2i coord, int range, bool avoidInvalid = false, bool includeSource = false) const;
	void getTileRadius(std::vector<const Tile*>& tileArea, sf::Vector2i coord, int range, bool avoidInvalid = false, bool includeSource = false) const;
	
	//Returns tiles in two cones emanating from a given tile, skipping the tile itself
	std::vector<const Tile*> getTileCone(sf::Vector2i coord, int range, eDirection direction, bool avoidInvalid = false) const;
	void getTileCone(std::vector<const Tile*>& tileArea, sf::Vector2i coord, int range, eDirection direction, bool avoidInvalid = false) const;

	//Returns tiles in a line from a given direction,
	//An element in the vector will be nullptr if it accesses an invalid tile
	std::vector<const Tile*> getTileLine(sf::Vector2i coord, int range, eDirection direction, bool avoidInvalid = false)const;
	void getTileLine(std::vector<const Tile*>& tileArea, sf::Vector2i coord, int range, eDirection direction, bool avoidInvalid = false) const;

	//Returns a ring of tiles at a certain radius from a specified tile
	//An element in the vector will be nullptr if it accesses an invalid tile
	std::vector<const Tile*> getTileRing(sf::Vector2i coord, int range)const;
	void getTileRing(std::vector<const Tile*>& tileArea, sf::Vector2i coord, int range) const;

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

	void renderMap(sf::RenderWindow& window);
	sf::Vector2i getDrawOffset() const { return m_drawOffset; }
	void setDrawOffset(sf::Vector2i newOffset) { m_drawOffset = newOffset; }

	sf::Vector2i getDimensions() const { return m_mapDimensions; }

	float getDrawScale() const { return m_drawScale; }
	void setDrawScale(float scale) { if (scale > 0.0) m_drawScale = scale; }

	float getWindStrength() const { return m_windStrength; }
	void setWindStrength(float strength) { if (strength > 0.0) m_windStrength = strength; }

	eDirection getWindDirection() const { return m_windDirection; }
	void setWindDirection(eDirection direction) { m_windDirection = direction; }

	void loadmap(const std::string& mapName);

	const std::vector<Tile>& getData()const { return m_data; }

	bool isTileCollidable(const Tile* tile) const;

	Map();
	Map(const Map&) = delete;
	Map& operator=(const Map&) = delete;
	Map(Map&&) = delete;
	Map&& operator=(Map&&) = delete;
};