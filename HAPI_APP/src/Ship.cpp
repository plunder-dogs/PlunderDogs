 #include "Ship.h"
#include "Map.h"
#include "PathFinding.h"
#include "Resources.h"
#include "GameEventMessenger.h"
#include "Utilities/Utilities.h"
#include "Battle.h"
//Debug
#include <iostream>


enum class eShipSpriteFrame
{
	eMaxHealth = 0,
	eLowDamage,
	eHighDamage,
	eDead
};

constexpr float MOVEMENT_ANIMATION_TIME(0.35f);
constexpr int ROTATION_ANGLE = 60;
constexpr size_t MOVEMENT_GRAPH_SIZE{ 32 };

const std::deque<Ray2D>& Ship::getMovementArea() const
{
	return m_movementArea.m_tileArea;
}

sf::FloatRect Ship::getAABB(const Map& map) const
{
	sf::Vector2i position = map.getTileScreenPos(m_currentPosition);
	sf::FloatRect AABB(sf::Vector2f(position.x, position.y), m_sprite.getSize());
	return AABB;
}

eFactionName Ship::getFactionName() const
{
	return m_factionName;
}

eDirection Ship::getCurrentDirection() const
{
	return m_currentDirection;
}

eShipType Ship::getShipType() const
{
	return m_shipType;
}

sf::Vector2i Ship::getCurrentPosition() const
{
	return m_currentPosition;
}

bool Ship::isWeaponFired() const
{
	return m_weaponFired;
}

bool Ship::isDead() const
{
	return m_isDead;
}

bool Ship::isMovingToDestination() const
{
	return m_movingToDestination;
}

bool Ship::isDestinationSet() const
{
	return m_destinationSet;
}

bool Ship::isDeployed() const
{
	return m_deployed;
}

bool Ship::isDeploymentStarted() const
{
	return m_deploymentStarted;
}

int Ship::getMovementPoints() const
{
	return m_movementPoints;
}

int Ship::getRange() const
{
	return m_range;
}

int Ship::getDamage() const
{
	return m_damage;
}

int Ship::getHealth() const
{
	return m_health;
}

int Ship::getID() const
{
	return m_ID;
}

void Ship::generateMovementArea(const Faction& faction, const Map & map, sf::Vector2i destination, bool displayOnlyLastPosition)
{
	if (isMovingToDestination() || isDestinationSet() || m_isDead)
	{
		return;
	}

	Ray2D start = { m_currentPosition, m_currentDirection };
	Ray2D end = { destination.x, destination.y };
	PathFinding::getInstance().findPath(m_movementArea, map, start, end, m_movementPoints);
	if (m_movementArea.m_tileArea.empty())
	{
		return;
	}

	m_movementArea.clearDisplayGraph();
	m_displayOnlyLastPosition = displayOnlyLastPosition;
	if (m_displayOnlyLastPosition)
	{
		sf::Vector2i lastPosition = m_movementArea.m_tileArea.back().pair();
		m_movementArea.m_tileAreaGraph[0].setPosition(lastPosition);
		m_movementArea.m_tileAreaGraph[0].activate();
	}
	else
	{
		m_movementArea.activateGraph();
	}
}

void Ship::rectifyMovementArea(const Faction & faction)
{
	//TODO: Not sure if it should be empty
	if (m_movementArea.m_tileArea.empty())
	{
		return;
	}
	//Disallow overlapping of destination for ships belonging to same Faction
	bool destinationOverlap = true;
	int nodeFromEnd = 1;
	while (destinationOverlap)
	{
		destinationOverlap = false;
		for (auto& ship : faction.getAllShips())
		{
			const auto& shipMovementArea = ship.getMovementArea();
			if (ship.getID() == m_ID || shipMovementArea.empty())
			{
				continue;
			}
		
			assert(!m_movementArea.m_tileArea.empty());
			//Found matching destination for two ships belonging to same Faction
			if (m_movementArea.m_tileArea.back().pair() == shipMovementArea.back().pair())
			{
				destinationOverlap = true;
				m_movementArea.m_tileArea.pop_back();

				if (m_movementArea.m_tileArea.empty())
				{
					return;
				}

				if (m_displayOnlyLastPosition)
				{
					sf::Vector2i lastPosition = m_movementArea.m_tileArea.back().pair();
					m_movementArea.m_tileAreaGraph[0].setPosition(lastPosition);
					m_movementArea.m_tileAreaGraph[0].activate();
				}
				break;
			}
		}
	}
}

void Ship::clearMovementArea()
{
	if (!m_movingToDestination)
	{
		m_movementArea.clearTileArea();
	}
}

void Ship::enableAction()
{
	if (!m_isDead)
	{
		m_actionSprite.activate();
	}
}

void Ship::disableAction()
{
	m_actionSprite.deactivate();

}

void Ship::startMovement(Map& map)
{
	//No adaquete area to move to
	if (m_movementArea.m_tileArea.empty())
	{
		return;
	}

	if (!m_destinationSet)
	{
		m_destinationSet = true;
		m_movingToDestination = true;

		assert(!m_movementArea.m_tileArea.empty());
		map.updateShipOnTile({ m_factionName, m_ID }, m_currentPosition,
			sf::Vector2i(m_movementArea.m_tileArea.back().x, m_movementArea.m_tileArea.back().y));

		m_actionSprite.deactivate();
	}
}

void Ship::startMovement(Map& map, eDirection endDirection)
{
	//No adaquete area to move to
	if (m_movementArea.m_tileArea.empty())
	{
		return;
	}

	if (!m_destinationSet)
	{
		m_destinationSet = true;
		m_movingToDestination = true;

		assert(!m_movementArea.m_tileArea.empty());
		m_movementArea.m_tileArea.emplace_back(m_movementArea.m_tileArea.back().pair(), endDirection);

		map.updateShipOnTile({ m_factionName, m_ID }, m_currentPosition,
			sf::Vector2i(m_movementArea.m_tileArea.back().x, m_movementArea.m_tileArea.back().y));

		m_actionSprite.deactivate();
	}
}

void Ship::takeDamage(int damageAmount)
{
	m_health -= damageAmount;
	int healthPercentage = ((float)m_health / m_maxHealth) * 100;
	if (healthPercentage < 100 && healthPercentage >= 50)
	{
		m_sprite.setFrameID(static_cast<int>(eShipSpriteFrame::eLowDamage));
	}
	else if (healthPercentage < 50 && healthPercentage >= 1)
	{
		m_sprite.setFrameID(static_cast<int>(eShipSpriteFrame::eHighDamage));
	}
	else
	{
		m_health = 0;
		m_isDead = true;
		m_sprite.setFrameID(static_cast<int>(eShipSpriteFrame::eDead));
		m_actionSprite.deactivate();

		FactionShipDestroyedEvent shipDestroyedEvent(m_factionName, m_ID);
		GameEventMessenger::getInstance().broadcast(GameEvent(&shipDestroyedEvent), eGameEvent::eFactionShipDestroyed);
	}
}

void Ship::fireWeapon()
{
	m_weaponFired = true;
	m_actionSprite.deactivate();
}

void Ship::setDestination()
{
	m_destinationSet = true;
}

void Ship::onNewBattlePhase(GameEvent gameEvent)
{
	m_weaponFired = false;
	m_destinationSet = false;
	m_movingToDestination = false;
}

Ship::Ship(eFactionName factionName, eShipType shipType, int ID)
	: m_factionName(factionName),
	m_shipType(shipType),
	m_ID(ID),
	m_currentPosition(),
	m_movementTimer(MOVEMENT_ANIMATION_TIME),
	m_movementPathSize(0),
	m_currentDirection(),
	m_weaponFired(false),
	m_isDead(false),
	m_actionSprite(),
	m_movingToDestination(false),
	m_destinationSet(false),
	m_displayOnlyLastPosition(false),
	m_deploymentStarted(false),
	m_maxHealth(0),
	m_health(0),
	m_damage(0),
	m_range(0),
	m_movementPoints(0),
	m_sprite(),
	m_deployed(false),
	m_movementArea(Textures::getInstance().getTexture(SPAWN_HEX), MOVEMENT_GRAPH_SIZE)
{
	//Action Sprite
	m_actionSprite.setScale(sf::Vector2f(2.0f, 2.0f));
	m_actionSprite.deactivate();

	switch (factionName)
	{
	case eFactionName::eYellow:
		m_actionSprite.setTexture(Textures::getInstance().getTexture(YELLOW_SPAWN_HEX));
		break;
	case eFactionName::eBlue:
		m_actionSprite.setTexture(Textures::getInstance().getTexture(BLUE_SPAWN_HEX));
		break;
	case eFactionName::eGreen:
		m_actionSprite.setTexture(Textures::getInstance().getTexture(GREEN_SPAWN_HEX));
		break;
	case eFactionName::eRed:
		m_actionSprite.setTexture(Textures::getInstance().getTexture(RED_SPAWN_HEX));
		break;
	}

	//Initialize Ship
	switch (shipType)
	{
	case eShipType::eFrigate:
		m_movementPoints = 25;
		m_maxHealth = 5;
		m_health = 5;
		m_range = 5;
		m_damage = 5;
		break;
	case eShipType::eTurtle:
		m_movementPoints = 8;
		m_maxHealth = 20;
		m_health = 20;
		m_range = 1;
		m_damage = 2;
		break;
	case eShipType::eFire:
		m_maxHealth = 8;
		m_health = 8;
		m_movementPoints = 10;
		m_range = 2;
		m_damage = 6;
		break;
	case eShipType::eSniper:
		m_maxHealth = 8;
		m_health = 8;
		m_movementPoints = 6;
		m_range = 10;
		m_damage = 4;
		break;
	}

	switch (factionName)
	{
	case eFactionName::eYellow:
		switch (shipType)
		{
		case eShipType::eFrigate:
			m_sprite.setTexture(Textures::getInstance().getTexture(YELLOW_SHIP_SIDE_CANNONS));
			break;
		case eShipType::eTurtle:
			m_sprite.setTexture(Textures::getInstance().getTexture(YELLOW_SHIP_BOMB)); 
			break;
		case eShipType::eFire:
			m_sprite.setTexture(Textures::getInstance().getTexture(YELLOW_SHIP_MELEE));
			break;
		case eShipType::eSniper:
			m_sprite.setTexture(Textures::getInstance().getTexture(YELLOW_SHIP_SNIPE));
			break;
		}
		break;

	case eFactionName::eBlue:
		switch (shipType)
		{
		case eShipType::eFrigate:
			m_sprite.setTexture(Textures::getInstance().getTexture(BLUE_SHIP_SIDE_CANNONS));
			break;
		case eShipType::eTurtle:
			m_sprite.setTexture(Textures::getInstance().getTexture(BLUE_SHIP_BOMB));
			break;
		case eShipType::eFire:
			m_sprite.setTexture(Textures::getInstance().getTexture(BLUE_SHIP_MELEE)); 
			break;
		case eShipType::eSniper:
			m_sprite.setTexture(Textures::getInstance().getTexture(BLUE_SHIP_SNIPE));
			break;
		}
		break;
	case eFactionName::eRed:
		switch (shipType)
		{
		case eShipType::eFrigate:
			m_sprite.setTexture(Textures::getInstance().getTexture(RED_SHIP_SIDE_CANNONS));
			break;
		case eShipType::eTurtle:
			m_sprite.setTexture(Textures::getInstance().getTexture(RED_SHIP_BOMB));
			break;
		case eShipType::eFire:
			m_sprite.setTexture(Textures::getInstance().getTexture(RED_SHIP_MELEE)); 
			break;
		case eShipType::eSniper:
			m_sprite.setTexture(Textures::getInstance().getTexture(RED_SHIP_SNIPE));
			break;
		default:
			break;
		}
		break;
	case eFactionName::eGreen:
		switch (shipType)
		{
		case eShipType::eFrigate:
			m_sprite.setTexture(Textures::getInstance().getTexture(GREEN_SHIP_SIDE_CANNONS));
			break;
		case eShipType::eTurtle:
			m_sprite.setTexture(Textures::getInstance().getTexture(GREEN_SHIP_BOMB)); 
			break;
		case eShipType::eFire:
			m_sprite.setTexture(Textures::getInstance().getTexture(GREEN_SHIP_MELEE));
			break;
		case eShipType::eSniper:
			m_sprite.setTexture(Textures::getInstance().getTexture(GREEN_SHIP_SNIPE));
			break;
		}
		break;
	}

	GameEventMessenger::getInstance().subscribe(std::bind(&Ship::onNewBattlePhase, this, std::placeholders::_1), eGameEvent::eEnteredNewBattlePhase);
	m_sprite.setFrameID(static_cast<int>(eShipSpriteFrame::eMaxHealth));
	m_sprite.setOriginAtCenter();
}

Ship::~Ship()
{
	GameEventMessenger::getInstance().unsubscribe(eGameEvent::eEnteredNewBattlePhase);
}

void Ship::update(float deltaTime)
{
	if (!m_movementArea.m_tileArea.empty() && !isDead() && m_movingToDestination)
	{
		m_movementTimer.update(deltaTime);
		if (m_movementTimer.isExpired())
		{
			m_movementTimer.reset();
			m_currentPosition = m_movementArea.m_tileArea.front().pair();
			m_movementArea.disableNode(m_currentPosition);
			
			int directionToTurn = static_cast<int>(m_movementArea.m_tileArea.front().dir);
			m_sprite.setRotation(directionToTurn * ROTATION_ANGLE % 360);
			m_currentDirection = (eDirection)directionToTurn;

			m_movementArea.m_tileArea.pop_front();

			if (m_movementArea.m_tileArea.empty())
			{
				m_movingToDestination = false;
				clearMovementArea();
			}
		}
	}
}

void Ship::render(sf::RenderWindow& window, const Map & map)
{
	//Render Ship
	const sf::Vector2i tileTransform = map.getTileScreenPos(m_currentPosition);
	float scale = map.getDrawScale();
	m_sprite.setPosition(sf::Vector2i(
		static_cast<float>(tileTransform.x + DRAW_OFFSET_X * scale),
		static_cast<float>(tileTransform.y + DRAW_OFFSET_Y * scale) ));
	m_sprite.setScale({ scale / 2, scale / 2 });
	m_sprite.render(window);

	m_actionSprite.setPosition(m_currentPosition);
	m_actionSprite.render(window, map);
}

void Ship::renderMovementArea(sf::RenderWindow & window, const Map & map)
{
	m_movementArea.render(window, map);
}

void Ship::setDeploymentPosition(sf::Vector2i position, eDirection direction)
{
	m_deploymentStarted = true;
	m_currentPosition = position;
	m_sprite.setRotation(static_cast<int>(direction) * ROTATION_ANGLE % 360);
}

void Ship::deployAtPosition(sf::Vector2i position, eDirection startingDirection)
{
	m_currentPosition = position;
	m_deployed = true;
	m_sprite.setRotation(static_cast<int>(startingDirection) * ROTATION_ANGLE % 360);
}