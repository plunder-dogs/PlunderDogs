#pragma once

#include "Global.h"
#include <memory>
#include <vector>
#include "Sprite.h"
#include <deque>

class Map;
struct Texture;
struct Ray2DArea
{
	Ray2DArea(const Texture& texture, size_t maxTileAreaSize, bool activeGraph = false);

	void disableNode(sf::Vector2i position);
	
	void render(sf::RenderWindow& window, const Map& map);
	void clearTileArea();
	void clearDisplayGraph();
	void activateGraph();

	std::vector<Sprite> m_tileAreaGraph;
	std::deque<Ray2D> m_tileArea;
};

struct Tile;
struct TileArea
{
	TileArea(size_t maxTileAreaSize);
	TileArea(const Texture& texture, size_t maxTileAreaSize, bool activeGraph = false);

	bool isPositionInTileArea(sf::Vector2i position) const;

	void render(sf::RenderWindow& window, const Map& map);
	void clearTileArea();
	void activateGraph();

	std::vector<Sprite> m_tileAreaGraph;
	std::vector<const Tile*> m_tileArea;
};