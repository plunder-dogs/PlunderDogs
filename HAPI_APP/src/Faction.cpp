#include "Faction.h"
#include <assert.h>
#include "Textures.h"
#include "Map.h"

constexpr size_t MAX_SPAWN_AREA = 75;

//BATTLE PLAYER
Faction::Faction(FactionName factionName, ePlayerType playerType)
	: m_ships(),
	m_factionName(factionName),
	m_playerType(playerType),
	m_spawnArea(MAX_SPAWN_AREA)
{
	m_ships.reserve(size_t(6));
}

const Ship & Faction::getShip(int shipID) const
{
	assert(shipID < m_ships.size());
	return m_ships[shipID];
}

void Faction::render(sf::RenderWindow& window, const Map & map, BattlePhase currentBattlePhase)
{
	for (auto& spawnArea : m_spawnArea.m_tileAreaGraph)
	{
		spawnArea.render(window, map);
	}

	if (currentBattlePhase == BattlePhase::Deployment)
	{
		for (auto& ship : m_ships)
		{
			if (ship.isDeployed())
			{
				ship.render(window, map);
			}
			else
			{
				ship.render(window, map);
				break;
			}
		}		
	}
	else
	{
		for (auto& ship : m_ships)
		{
			ship.render(window, map);
		}
	}
}

void Faction::addShip(FactionName factionName, eShipType shipType)
{
	assert(m_ships.size() < size_t(6));
	int shipID = static_cast<int>(m_ships.size());

	m_ships.emplace_back(factionName, shipType, shipID);
}

bool Faction::isAllShipsDeployed() const
{
	for (const auto& ship : m_ships)
	{
		if (!ship.isDeployed())
		{
			return false;
		}
	}

	return true;
}

void Faction::createSpawnArea(Map & map)
{
	const Texture* texture = nullptr;
	switch (m_factionName)
	{
	case eYellow:
		texture = Textures::getInstance().m_yellowSpawnHex.get();
		break;
	case eBlue:
		texture = Textures::getInstance().m_blueSpawnHex.get();
		break;
	case eGreen:
		texture = Textures::getInstance().m_greenSpawnHex.get();
		break;
	case eRed:
		texture = Textures::getInstance().m_redSpawnHex.get();
		break;
	};

	assert(texture);
	map.getTileRadius(m_spawnArea.m_tileArea, map.getRandomSpawnPosition(), 3, true, true);
	for (const auto& tile : m_spawnArea.m_tileArea)
	{
		m_spawnArea.m_tileAreaGraph.emplace_back(*texture, tile->m_tileCoordinate, true);
		m_spawnArea.m_tileAreaGraph.back().setScale(sf::Vector2f(2.0f, 2.0f));
	}
}

bool Faction::deployShipAtPosition(Map& map, sf::Vector2i startingPosition, eDirection startingDirection)
{
	auto cIter = std::find_if(m_spawnArea.m_tileArea.cbegin(), m_spawnArea.m_tileArea.cend(),
		[startingPosition](const auto& tile) { return startingPosition == tile->m_tileCoordinate; });
	if (cIter != m_spawnArea.m_tileArea.cend())
	{
		for (auto& ship : m_ships)
		{
			if (!ship.isDeployed())
			{
				ship.deployAtPosition(startingPosition, startingDirection);
				map.setShipOnTile({ m_factionName, ship.getID() }, startingPosition);
				break;
			}
		}

		return true;
	}
	else
	{
		return false;
	}
}

void Faction::setShipDeploymentAtPosition(sf::Vector2i startingPosition)
{
	auto cIter = std::find_if(m_spawnArea.m_tileArea.cbegin(), m_spawnArea.m_tileArea.cend(),
		[startingPosition](const auto& tile) { return startingPosition == tile->m_tileCoordinate; });
	for (auto& ship : m_ships)
	{
		if (!ship.isDeployed())
		{
			ship.setDeploymentPosition(startingPosition);
			break;
		}
	}
}

bool Faction::isEliminated() const
{
	bool allShipsDestroyed = true;
	for (const auto& ship : m_ships)
	{
		if (!ship.isDead())
		{
			allShipsDestroyed = false;
			break;
		}
	}

	return allShipsDestroyed;
}

void Faction::shipTakeDamage(int shipID, int damage)
{
	assert(static_cast<size_t>(shipID) <= m_ships.size());
	m_ships[shipID].takeDamage(damage);
}

bool Faction::moveShipToPosition(Map& map, int shipID, sf::Vector2i destination)
{
	assert(static_cast<size_t>(shipID) <= m_ships.size());
	return m_ships[shipID].move(map, destination);
}

bool Faction::moveShipToPosition(Map& map, int shipID, sf::Vector2i destination, eDirection endDirection)
{
	assert(static_cast<size_t>(shipID) <= m_ships.size());
	return m_ships[shipID].move(map, destination, endDirection);
}

void Faction::generateShipMovementGraph(const Map & map, int shipID, sf::Vector2i destination)
{
	assert(static_cast<size_t>(shipID) <= m_ships.size());
	m_ships[shipID].generateMovementGraph(map, destination);
}

void Faction::disableShipMovementGraph(int shipID)
{
	assert(static_cast<size_t>(shipID) <= m_ships.size());
	m_ships[shipID].disableMovementGraph();
}