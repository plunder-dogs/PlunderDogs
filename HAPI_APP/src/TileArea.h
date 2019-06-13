#pragma once

#include <memory>
#include <vector>
#include <Sprite.h>

struct Tile;
class Map;
struct Texture;
struct TileArea
{
	TileArea(size_t maxTileAreaSize);
	TileArea(std::unique_ptr<Texture>& texture, size_t maxTileAreaSize, const Map& map);

	void render(sf::RenderWindow& window, const Map& map);
	void clearTileArea();

	bool m_display;
	std::vector<Sprite> m_tileAreaGraph;
	std::vector<const Tile*> m_tileArea;
};