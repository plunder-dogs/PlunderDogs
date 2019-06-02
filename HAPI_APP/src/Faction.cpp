#include "Faction.h"
#include <assert.h>
#include "Textures.h"
#include "Map.h"

//BATTLE PLAYER
Faction::Faction(FactionName name, ePlayerType playerType)
	: m_ships(),
	m_factionName(name),
	m_playerType(playerType),
	m_eliminated(false),
	m_shipToDeploy(nullptr),
	m_spawnArea()
{
	m_ships.reserve(size_t(6));
}

const Ship & Faction::getShip(int shipID) const
{
	assert(shipID < m_ships.size());
	return m_ships[shipID];
}

void Faction::render(sf::RenderWindow& window, const Map & map)
{
	for (auto& spawnArea : m_spawnArea)
	{
		spawnArea.render(window, map);
	}

	for (auto& ship : m_ships)
	{
		if (ship.isDeployed())
		{
			ship.render(window, map);
		}
	}

	if (m_shipToDeploy)
	{
		m_shipToDeploy->render(window, map);
	}
}

void Faction::addShip(FactionName factionName, eShipType shipType)
{
	assert(m_ships.size() < size_t(6));
	int shipID = static_cast<int>(m_ships.size());
	m_ships.emplace_back(factionName, shipType, shipID);

	if (!m_shipToDeploy)
	{
		m_shipToDeploy = &m_ships.back();
	}
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
	std::vector<const Tile*> tileRadius = map.cGetTileRadius(map.getRandomSpawnPosition(), 3, true, true);
	m_spawnArea.reserve(tileRadius.size());
	for (const auto& tile : tileRadius)
	{
		m_spawnArea.emplace_back(m_factionName, tile->m_tileCoordinate);
	}
}

bool Faction::deployShipAtPosition(Map& map, sf::Vector2i startingPosition, eDirection startingDirection)
{
	assert(m_shipToDeploy);
	auto cIter = std::find_if(m_spawnArea.cbegin(), m_spawnArea.cend(),
		[startingPosition](const auto& spawnArea) { return startingPosition == spawnArea.m_position; });
	if (cIter != m_spawnArea.cend())
	{
		m_shipToDeploy->deployAtPosition(startingPosition, startingDirection);
		map.setShipOnTile({ m_factionName, m_shipToDeploy->getID() }, startingPosition);
		//Select new Ship
		for (auto& ship : m_ships)
		{
			if (!ship.isDeployed())
			{
				m_shipToDeploy = &ship;
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

bool Faction::setShipDeploymentAtPosition(sf::Vector2i startingPosition)
{
	if (!m_shipToDeploy)
	{
		return false;
	}
	auto cIter = std::find_if(m_spawnArea.cbegin(), m_spawnArea.cend(),
		[startingPosition](const auto& spawnArea) { return startingPosition == spawnArea.m_position; });
	
	m_shipToDeploy->setDeploymentPosition(startingPosition);
	return cIter != m_spawnArea.end();
}

void Faction::onNewTurn()
{
	for (auto& ship : m_ships)
	{
		ship.onNewTurn();
	}
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

void Faction::generateShipMovementPath(const Map & map, int shipID, sf::Vector2i destination)
{
	assert(static_cast<size_t>(shipID) <= m_ships.size());
	m_ships[shipID].generateMovementPath(map, destination);
}

void Faction::disableShipMovementPath(int shipID)
{
	assert(static_cast<size_t>(shipID) <= m_ships.size());
	m_ships[shipID].disableMovementPath();
}

SpawnNode::SpawnNode(FactionName factionName, sf::Vector2i position)
	: m_position(position),
	m_sprite()
{
	switch (factionName)
	{
	case eYellow:
		m_sprite.setTexture(*Textures::m_yellowSpawnHex);
		break;
	case eBlue:
		m_sprite.setTexture(*Textures::m_blueSpawnHex);
		break;
	case eGreen:
		m_sprite.setTexture(*Textures::m_greenSpawnHex);
		break;
	case eRed:
		m_sprite.setTexture(*Textures::m_redSpawnHex);
		break;
	};

	m_sprite.setScale(sf::Vector2f(2.f, 2.f));
	//m_sprite->GetTransformComp().SetOriginToCentreOfFrame();
}

void SpawnNode::render(sf::RenderWindow& window, const Map & map)
{
	sf::Vector2i screenPosition = map.getTileScreenPos(m_position);
	m_sprite.setPosition(
	(float)screenPosition.x + DRAW_OFFSET_X * map.getDrawScale(),
	(float)screenPosition.y + DRAW_OFFSET_Y * map.getDrawScale() );
	window.draw(m_sprite);
}