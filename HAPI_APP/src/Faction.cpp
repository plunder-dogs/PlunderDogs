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
	m_ships.reserve(MAX_SHIPS_PER_FACTION);
}

bool Faction::isPositionInSpawnArea(sf::Vector2i position) const
{
	return m_spawnArea.isPositionInTileArea(position);
}

const std::vector<Ship>& Faction::getAllShips() const
{
	return m_ships;
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

	if (currentBattlePhase == BattlePhase::Deployment && m_playerType == ePlayerType::eHuman)
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
	else if(currentBattlePhase != BattlePhase::Deployment)
	{
		for (auto& ship : m_ships)
		{
			ship.render(window, map);
		}
	}
}

void Faction::renderShipsMovementGraphs(sf::RenderWindow & window, const Map & map)
{
	for (auto& ship : m_ships)
	{
		ship.renderMovementArea(window, map);
	}
}

bool Faction::isPositionInDeploymentArea(sf::Vector2i position) const
{
	auto cIter = std::find_if(m_spawnArea.m_tileArea.cbegin(), m_spawnArea.m_tileArea.cend(),
		[position](const auto& tile) { return position == tile->m_tileCoordinate; });
	return cIter != m_spawnArea.m_tileArea.cend();
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

void Faction::deployShipAtPosition(Map& map, sf::Vector2i startingPosition, eDirection startingDirection)
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
	}
}

void Faction::setShipDeploymentAtPosition(sf::Vector2i startingPosition, eDirection direction)
{
	for (auto& ship : m_ships)
	{
		if (!ship.isDeployed())
		{
			ship.setDeploymentPosition(startingPosition, direction);
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

void Faction::clearSpawnArea()
{
	m_spawnArea.m_tileAreaGraph.clear();
	m_spawnArea.m_tileArea.clear();
}

void Faction::shipTakeDamage(int shipID, int damage)
{
	assert(static_cast<size_t>(shipID) <= m_ships.size());
	m_ships[shipID].takeDamage(damage);
}

void Faction::moveShipToPosition(Map& map, int shipID)
{
	assert(static_cast<size_t>(shipID) <= m_ships.size());
	m_ships[shipID].startMovement(map);
}

void Faction::moveShipToPosition(Map& map, int shipID, eDirection endDirection)
{
	assert(static_cast<size_t>(shipID) <= m_ships.size());
	m_ships[shipID].startMovement(map, endDirection);
}

void Faction::generateShipMovementArea(const Map & map, int shipID, sf::Vector2i destination)
{
	assert(static_cast<size_t>(shipID) <= m_ships.size());
	m_ships[shipID].generateMovementArea(map, destination);
}

void Faction::clearShipMovementArea(int shipID)
{
	assert(static_cast<size_t>(shipID) <= m_ships.size());
	m_ships[shipID].clearMovementArea();
}