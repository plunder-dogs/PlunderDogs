#include "TileArea.h"
#include "Map.h"
#include "Texture.h"
#include <assert.h>

TileArea::TileArea(size_t maxTileAreaSize)
	: m_tileAreaGraph(),
	m_tileArea()
{
	m_tileArea.reserve(maxTileAreaSize);
	m_tileAreaGraph.reserve(maxTileAreaSize);
}

TileArea::TileArea(std::unique_ptr<Texture>& texture, size_t maxTileAreaSize, const Map& map, bool activeGraph)
	: m_tileAreaGraph(),
	m_tileArea()
{
	m_tileArea.reserve(maxTileAreaSize);
	m_tileAreaGraph.reserve(maxTileAreaSize);

	float scale = map.getDrawScale();
	for (int i = 0; i < maxTileAreaSize; ++i)
	{
		m_tileAreaGraph.emplace_back(texture, sf::Vector2f(2, 2), activeGraph);
	}
}

void TileArea::render(sf::RenderWindow & window, const Map & map)
{
	assert(m_tileArea.size() <= m_tileAreaGraph.size());

	for (int i = 0; i < m_tileArea.size(); i++)
	{
		m_tileAreaGraph[i].render(window, map);
	}
}

void TileArea::clearTileArea()
{
	m_tileArea.clear();

	for (auto& tile : m_tileAreaGraph)
	{
		tile.deactivate();
	}
}

void TileArea::activateGraph()
{
	assert(m_tileArea.size() <= m_tileAreaGraph.size());

	for (int i = 0; i < m_tileArea.size(); i++)
	{
		assert(m_tileArea[i]);
		m_tileAreaGraph[i].setPosition(m_tileArea[i]->m_tileCoordinate);
		m_tileAreaGraph[i].activate();
	}
}