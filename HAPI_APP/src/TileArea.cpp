#include "TileArea.h"
#include "Map.h"
#include "Texture.h"
#include <assert.h>

TileArea::TileArea(size_t maxTileAreaSize)
	: m_display(false),
	m_tileAreaGraph(),
	m_tileArea()
{
	m_tileArea.reserve(maxTileAreaSize);
	m_tileAreaGraph.reserve(maxTileAreaSize);
}

TileArea::TileArea(std::unique_ptr<Texture>& texture, size_t maxTileAreaSize, const Map& map)
	: m_display(false),
	m_tileAreaGraph(),
	m_tileArea()
{
	m_tileArea.reserve(maxTileAreaSize);
	m_tileAreaGraph.reserve(maxTileAreaSize);

	float scale = map.getDrawScale();
	for (int i = 0; i < maxTileAreaSize; ++i)
	{
		m_tileAreaGraph.emplace_back(texture, sf::Vector2f(scale, scale));
	}
}

void TileArea::render(sf::RenderWindow & window, const Map & map)
{
	assert(m_tileArea.size() <= m_tileAreaGraph.size());

	if (m_display)
	{
		float scale = map.getDrawScale();
		for (int i = 0; i < m_tileArea.size(); i++)
		{
			posi pos = map.getTileScreenPos(m_tileArea[i]->m_tileCoordinate);
			float x = static_cast<float>(pos.x) + DRAW_OFFSET_X * scale;
			float y = static_cast<float>(pos.y) + DRAW_OFFSET_Y * scale;
			m_tileAreaGraph[i].setPosition(sf::Vector2i(x, y));
			m_tileAreaGraph[i].render(window);
		}
	}
}

void TileArea::clearTileArea()
{
	m_tileArea.clear();
	m_display = false;
}