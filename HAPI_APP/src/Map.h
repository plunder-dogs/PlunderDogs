#pragma once
#include <utility>
#include <vector>
#include <string>
#include <HAPISprites_lib.h>
#include <HAPISprites_UI.h>
#include "global.h"

constexpr float DRAW_OFFSET_X{ 12 };
constexpr float DRAW_OFFSET_Y{ 28 };

//Info for HAPI_SPRITES - Leave for now. 
/*
//	//m_sprite->GetSpritesheet()->GenerateNormals(true);
**** HAPI Sprites: ERROR Detected
** Description: H_INVALID_PARAMETER - passed parameter was not valid
** System: HAPI Sprites
** Detail: surface width does not divide equally by numFrames
****
*/
struct BattleEntity;
struct Tile
{
	const enum eTileType m_type;
	//TODO: Dangerous exposure of raw pointer
	BattleEntity* m_entityOnTile;
	std::unique_ptr<HAPISPACE::Sprite> m_daySprite;
	std::unique_ptr<HAPISPACE::Sprite> m_aftersprite;
	std::unique_ptr<HAPISPACE::Sprite> m_eveningSprite;
	std::unique_ptr<HAPISPACE::Sprite> m_nightSprite;
	const std::pair<int, int> m_tileCoordinate;

	Tile(eTileType type, std::shared_ptr<HAPISPACE::SpriteSheet> daySpriteSheet,
						std::shared_ptr<HAPISPACE::SpriteSheet> afternoonSpriteSheet,
						std::shared_ptr<HAPISPACE::SpriteSheet> eveningSpriteSheet,
						std::shared_ptr<HAPISPACE::SpriteSheet> nightSpriteSheet,
						std::pair<int, int> coord) :
		m_type(type),
		m_entityOnTile(nullptr),
		m_daySprite(),
		m_aftersprite(),
		m_eveningSprite(),
		m_nightSprite(),
		m_tileCoordinate(coord)
	{
		//HAPI's make sprite takes a pointer to an existing spritesheet
		m_daySprite = HAPI_Sprites.MakeSprite(daySpriteSheet);
		m_aftersprite = HAPI_Sprites.MakeSprite(afternoonSpriteSheet);
		m_eveningSprite = HAPI_Sprites.MakeSprite(eveningSpriteSheet);
		m_nightSprite = HAPI_Sprites.MakeSprite(nightSpriteSheet);
	}
};

class Map
{
	struct SpawnPosition
	{
		SpawnPosition(std::pair<int, int> spawnPosition);

		std::pair<int, int> position;
		bool inUse;
	};
private:
	std::pair<int, int> m_mapDimensions;
	float m_windStrength;
	eDirection m_windDirection;

	float m_drawScale;
	std::pair<int, int> m_drawOffset;
	std::vector<Tile> m_data;
	std::vector<SpawnPosition> m_spawnPositions;

	std::pair<int, int> offsetToCube(std::pair<int, int> offset) const;
	std::pair<int, int> cubeToOffset(std::pair<int, int> cube) const;
	int cubeDistance(std::pair<int, int> a, std::pair<int, int> b) const;
	bool inCone(std::pair<int, int> orgHex, std::pair<int, int> testHex, eDirection dir) const;
	//Finds the euclidean distance from a point to a tile's centre, used by getMouseClickCoord
	float tileDistanceMag(std::pair<int, int> tileCoord, std::pair<int, int> mouseClick) const;
	void onReset();
public:
	//Returns a pointer to a given tile, returns nullptr if there is no tile there
	Tile* getTile(std::pair<int, int> coordinate);
	const Tile* getTile(std::pair<int, int> coordinate) const;
	Tile* getTile(posi coordinate);
	const Tile* getTile(posi coordinate) const;
	//An n = 1 version of getTileRadius for use in pathfinding, 
	//returns nullptr for each tile out of bounds
	std::vector<Tile*> getAdjacentTiles(std::pair<int, int> coord);
	std::vector<const Tile*> cGetAdjacentTiles(std::pair<int, int> coord) const;
	//Returns tiles in a radius around a given tile, skipping the tile itself
	std::vector<Tile*> getTileRadius(std::pair<int, int> coord, int range, bool avoidInvalid = false, bool includeSource = false);
	std::vector<const Tile*> cGetTileRadius(std::pair<int, int> coord, int range, bool avoidInvalid = false, bool includeSource = false) const;
	//Returns tiles in two cones emanating from a given tile, skipping the tile itself
	std::vector<Tile*> getTileCone(std::pair<int, int> coord, int range, eDirection direction, bool avoidInvalid = false);
	std::vector<const Tile*> cGetTileCone(std::pair<int, int> coord, int range, eDirection direction, bool avoidInvalid = false)const;
	//Returns tiles in a line from a given direction,
	//An element in the vector will be nullptr if it accesses an invalid tile
	std::vector<Tile*> getTileLine(std::pair<int, int> coord, int range, eDirection direction, bool avoidInvalid = false);
	std::vector<const Tile*> cGetTileLine(std::pair<int, int> coord, int range, eDirection direction, bool avoidInvalid = false)const;
	//Returns a ring of tiles at a certain radius from a specified tile
	//An element in the vector will be nullptr if it accesses an invalid tile
	std::vector<Tile*> getTileRing(std::pair<int, int> coord, int range);
	std::vector<const Tile*> cGetTileRing(std::pair<int, int> coord, int range)const;

	std::pair<int, int> getSpawnPosition();
	//For finding the location on the screen a given tile is being drawn
	std::pair<int, int> getTileScreenPos(std::pair<int, int> coord) const;

	//For finding the closest tile to the current mouse location, 
	//can give values that aren't valid tiles if you click off the map 
	//so check if getTile is null before using
	std::pair<int, int> getMouseClickCoord(std::pair<int, int> mouseCoord) const;

	//Moves an entitys position on the map, returns false if the position is already taken
	bool moveEntity(std::pair<int, int> originalPos, std::pair<int, int> newPos);
	//Places a new entity on the map (no check for duplicates yet so try to avoid creating multiples)
	void insertEntity(BattleEntity& newEntity);

	void drawMap(eLightIntensity lightIntensity) const;
	std::pair<int, int> getDrawOffset() const { return m_drawOffset; }
	void setDrawOffset(std::pair<int, int> newOffset) { m_drawOffset = newOffset; }

	std::pair<int, int> getDimensions() const { return m_mapDimensions; }

	float getDrawScale() const { return m_drawScale; }
	void setDrawScale(float scale) { if (scale > 0.0) m_drawScale = scale; }

	float getWindStrength() const { return m_windStrength; }
	void setWindStrength(float strength) { if (strength > 0.0) m_windStrength = strength; }

	eDirection getWindDirection() const { return m_windDirection; }
	void setWindDirection(eDirection direction) { m_windDirection = direction; }
	//TODO: Find out what this is
	//std::vector<std::pair<int, int>> getSpawnPositions() const { return m_spawnPositions; }

	void loadmap(const std::string& mapName);

	//Only for pathfinding
	const std::vector<Tile>& getData()const { return m_data; }

	Map();
	~Map();
};