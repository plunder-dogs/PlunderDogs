#include "TileArea.h"
#include "Map.h"
#include "Texture.h"
#include <assert.h>

Ray2DArea::Ray2DArea(const Texture& texture, size_t maxTileAreaSize, bool activeGraph)
{
	m_tileAreaGraph.reserve(maxTileAreaSize);

	for (int i = 0; i < maxTileAreaSize; ++i)
	{
		m_tileAreaGraph.emplace_back(texture, sf::Vector2i(2, 2), activeGraph);
	}
}

void Ray2DArea::disableNode(sf::Vector2i position)
{
	for (auto iter = m_tileAreaGraph.begin(); iter != m_tileAreaGraph.end(); ++iter)
	{
		if (iter->getPosition() == position)
		{
			iter->deactivate();
		}
	}
}

void Ray2DArea::render(sf::RenderWindow & window, const Map & map)
{
	assert(m_tileArea.size() <= m_tileAreaGraph.size());

	for (auto& i : m_tileAreaGraph)
	{
		i.render(window, map);
	}
}

void Ray2DArea::clearTileArea()
{
	assert(m_tileArea.size() <= m_tileAreaGraph.size());

	for (int i = 0; i < m_tileArea.size(); ++i)
	{
		m_tileAreaGraph[i].deactivate();
	}

	m_tileArea.clear();
}

void Ray2DArea::clearDisplayGraph()
{
	for (int i = 0; i < m_tileArea.size(); ++i)
	{
		m_tileAreaGraph[i].deactivate();
	}
}

void Ray2DArea::activateGraph()
{
	assert(m_tileArea.size() <= m_tileAreaGraph.size());

	for (int i = 0; i < m_tileArea.size(); i++)
	{
		m_tileAreaGraph[i].setPosition(sf::Vector2i(m_tileArea[i].x, m_tileArea[i].y));
		m_tileAreaGraph[i].activate();
	}
}

//TILE AREA
TileArea::TileArea(size_t maxTileAreaSize)
	: m_tileAreaGraph(),
	m_tileArea()
{
	m_tileArea.reserve(maxTileAreaSize);
	m_tileAreaGraph.reserve(maxTileAreaSize);
}

TileArea::TileArea(const Texture& texture, size_t maxTileAreaSize, bool activeGraph)
	: m_tileAreaGraph(),
	m_tileArea()
{
	m_tileArea.reserve(maxTileAreaSize);
	m_tileAreaGraph.reserve(maxTileAreaSize);

	for (int i = 0; i < maxTileAreaSize; ++i)
	{
		m_tileAreaGraph.emplace_back(texture, sf::Vector2i(2, 2), activeGraph);
	}
}

bool TileArea::isPositionInTileArea(sf::Vector2i position) const
{
	auto cIter = std::find_if(m_tileArea.cbegin(), m_tileArea.cend(), 
		[position](const auto& tile) { return tile->m_tileCoordinate == position; });

	return (cIter != m_tileArea.cend());
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
	for (auto& node : m_tileAreaGraph)
	{
		node.deactivate();
	}

	//for (int i = 0; i < m_tileArea.size(); ++i)
	//{
	//	m_tileAreaGraph[i].deactivate();
	//}

	m_tileArea.clear();
}

void TileArea::activateGraph()
{
	assert(m_tileArea.size() <= m_tileAreaGraph.size());

	for (int i = 0; i < m_tileArea.size(); i++)
	{
		m_tileAreaGraph[i].setPosition(m_tileArea[i]->m_tileCoordinate);
		m_tileAreaGraph[i].activate();
	}
}