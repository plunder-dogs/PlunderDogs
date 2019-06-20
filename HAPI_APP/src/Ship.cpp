#include "Ship.h"
#include "Map.h"
#include "BFS.h"
#include "Textures.h"
#include "GameEventMessenger.h"
#include "Utilities/Utilities.h"
#include "Battle.h"
//Debug
#include <iostream>

constexpr float MOVEMENT_ANIMATION_TIME(0.35f);
constexpr int ROTATION_ANGLE = 60;

FactionName Ship::getFactionName() const
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

void Ship::generateMovementArea(const Map & map, sf::Vector2i destination)
{
	if (isMovingToDestination() || isDestinationSet())
	{
		return;
	}

	posi start = { m_currentPosition, m_currentDirection };
	posi end = { destination.x, destination.y };
	std::queue<posi> pathToTile = BFS::findPath(map, start, end, m_movementPoints);
	if (pathToTile.empty())
	{
		return;
	}
	
	m_movementArea.clearTileArea();
	while (!pathToTile.empty())
	{
		m_movementArea.m_tileArea.push_back(pathToTile.front());
		pathToTile.pop();
	}
	
	m_movementArea.activateGraph();

	//int queueSize = static_cast<int>(pathToTile.size());
	//for (i; i < queueSize; ++i)
	//{
	//	m_movementGraph[i].activate();
	//	m_movementGraph[i].setPosition(pathToTile.front().pair());
	//	pathToTile.pop();
	//}
	//m_movementPathSize = i - 1;
}

void Ship::disableMovementGraph()
{
	m_movementArea.clearTileArea();
}

//sf::Vector2i Ship::getEndOfMovementPath() const
//{
//	//TODO: Refactor 
//
//	//int i = static_cast<int>(m_movementPath.size()) - 1;
//	//assert(i > 0);
//	//for (i; i > 0; i--)
//	//{
//	//	if (m_movementGraph[i].isActive())
//	//		break;
//	//}
//	//return m_movementGraph[i].getPosition();
//}

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

void Ship::startMovement(Map& map, sf::Vector2i destination)
{
	if (!m_destinationSet)
	{
		m_moving = true;
		map.updateShipOnTile({ m_factionName, m_ID }, m_currentPosition,
		sf::Vector2i(m_movementArea.m_tileArea.back().x, m_movementArea.m_tileArea.back().y));
		
		m_destinationSet = true;
		m_movingToDestination = true;
		m_actionSprite.deactivate();
	}
}

void Ship::startMovement(Map& map, sf::Vector2i destination, eDirection endDirection)
{
	//if (!m_destinationSet)
	//{
	//	posi currentPos = { m_currentPosition.x, m_currentPosition.y, m_currentDirection };
	//	posi destinationPos = { destination.x, destination.y };
	//	std::queue<posi> pathToTile = BFS::findPath(map, currentPos, destinationPos, m_movementPoints);
	//	if (!pathToTile.empty() && pathToTile.size() <= m_movementPathSize + 1)
	//	{
	//		pathToTile.emplace(posi(pathToTile.back().pair(), endDirection));
	//		m_movementPath = pathToTile;
	//		map.updateShipOnTile({ m_factionName, m_ID }, m_currentPosition, pathToTile.back().pair());
	//		m_destinationSet = true;
	//		m_movingToDestination = true;
	//		m_actionSprite.deactivate();
	//	}
	//}
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

void Ship::disableMovementGraphNode(sf::Vector2i position)
{
	m_movementArea.disableNode(position);
}

unsigned int Ship::getDirectionCost(int currentDirection, int newDirection)
{
	unsigned int diff = std::abs(newDirection - currentDirection);
	if (diff == 0)
	{
		return 0;
	}

	//number of direction % difference between the new and old directions
	return (static_cast<int>(eDirection::Max) % diff) + 1;
}

Ship::Ship(FactionName factionName, eShipType shipType, int ID)
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
	m_maxHealth(0),
	m_health(0),
	m_damage(0),
	m_range(0),
	m_movementPoints(0),
	m_sprite(),
	m_deployed(false),
	m_movementArea(Textures::getInstance().m_spawnHex, MOVEMENT_GRAPH_SIZE),
	m_moving(false)
{
	////Initialize Movement Path
	//for (auto& i : m_movementGraph)
	//{
	//	i.setTexture(Textures::getInstance().m_spawnHex);
	//	i.setScale(sf::Vector2f(0.5f, 0.5f));
	//	i.deactivate();
	//}
	
	//Action Sprite
	m_actionSprite.setScale(sf::Vector2f(2.0f, 2.0f));
	m_actionSprite.deactivate();

	switch (factionName)
	{
	case FactionName::eYellow:
		m_actionSprite.setTexture(Textures::getInstance().m_yellowSpawnHex);
		break;
	case FactionName::eBlue:
		m_actionSprite.setTexture(Textures::getInstance().m_blueSpawnHex);
		break;
	case FactionName::eGreen:
		m_actionSprite.setTexture(Textures::getInstance().m_greenSpawnHex);
		break;
	case FactionName::eRed:
		m_actionSprite.setTexture(Textures::getInstance().m_redSpawnHex);
		break;
	}

	//Initialize Ship
	switch (shipType)
	{
	case eShipType::eFrigate:
		m_movementPoints = 5;
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
	case FactionName::eYellow:
		switch (shipType)
		{
		case eShipType::eFrigate:
			m_sprite.setTexture(Textures::getInstance().m_yellowShipSideCannons); 
			break;
		case eShipType::eTurtle:
			m_sprite.setTexture(Textures::getInstance().m_yellowShipBomb);
			break;
		case eShipType::eFire:
			m_sprite.setTexture(Textures::getInstance().m_yellowShipMelee); 
			break;
		case eShipType::eSniper:
			m_sprite.setTexture(Textures::getInstance().m_yellowShipSnipe);
			break;
		}
		break;

	case FactionName::eBlue:
		switch (shipType)
		{
		case eShipType::eFrigate:
			m_sprite.setTexture(Textures::getInstance().m_blueShipSideCannons);
			break;
		case eShipType::eTurtle:
			m_sprite.setTexture(Textures::getInstance().m_blueShipBomb);
			break;
		case eShipType::eFire:
			m_sprite.setTexture(Textures::getInstance().m_blueShipMelee);
			break;
		case eShipType::eSniper:
			m_sprite.setTexture(Textures::getInstance().m_blueShipSnipe);
			break;
		}
		break;
	case FactionName::eRed:
		switch (shipType)
		{
		case eShipType::eFrigate:
			m_sprite.setTexture(Textures::getInstance().m_redShipSideCannons);
			break;
		case eShipType::eTurtle:
			m_sprite.setTexture(Textures::getInstance().m_redShipBomb);
			break;
		case eShipType::eFire:
			m_sprite.setTexture(Textures::getInstance().m_redShipMelee);
			break;
		case eShipType::eSniper:
			m_sprite.setTexture(Textures::getInstance().m_redShipSnipe);
			break;
		default:
			break;
		}
		break;
	case FactionName::eGreen:
		switch (shipType)
		{
		case eShipType::eFrigate:
			m_sprite.setTexture(Textures::getInstance().m_greenShipSideCannons);
			break;
		case eShipType::eTurtle:
			m_sprite.setTexture(Textures::getInstance().m_greenShipBomb);
			break;
		case eShipType::eFire:
			m_sprite.setTexture(Textures::getInstance().m_greenShipMelee);
			break;
		case eShipType::eSniper:
			m_sprite.setTexture(Textures::getInstance().m_greenShipSnipe);
			break;
		}
		break;
	}

	GameEventMessenger::getInstance().subscribe(std::bind(&Ship::onNewBattlePhase, this, std::placeholders::_1), eGameEvent::eEnteredNewBattlePhase);
	m_sprite.setFrameID(static_cast<int>(eShipSpriteFrame::eMaxHealth));
	m_sprite.setOriginAtCenter();

#ifdef HAPI_SPRITES
	m_sprite.GetTransformComp().SetOriginToCentreOfFrame();
	m_sprite.GetTransformComp().SetScaling({ 1, 1 });
#endif // SFML_REFACTOR
}

Ship::Ship(Ship & orig)
	: m_factionName(orig.m_factionName),
	m_shipType(orig.m_shipType),
	m_ID(orig.m_ID),
	m_currentPosition(),
	m_movementTimer(MOVEMENT_ANIMATION_TIME),
	m_movementPathSize(0),
	m_currentDirection(orig.m_currentDirection),
	m_weaponFired(false),
	m_isDead(false),
	m_actionSprite(orig.m_actionSprite),
	m_movingToDestination(false),
	m_destinationSet(false),
	m_maxHealth(orig.m_maxHealth),
	m_health(orig.m_health),
	m_damage(orig.m_damage),
	m_range(orig.m_range),
	m_movementPoints(orig.m_movementPoints),
	m_sprite(orig.m_sprite),
	m_deployed(false),
	m_movementArea(orig.m_movementArea)
{}

Ship::~Ship()
{
	GameEventMessenger::getInstance().unsubscribe(eGameEvent::eEnteredNewBattlePhase);
}

void Ship::update(float deltaTime)
{
	if (!m_movementArea.m_tileArea.empty() && !isDead() && m_moving)
	{
		m_movementTimer.update(deltaTime);
		if (m_movementTimer.isExpired())
		{
			m_movementTimer.reset();
			m_currentPosition = m_movementArea.m_tileArea.front().pair();
			disableMovementGraphNode(m_currentPosition);
		
			int directionToTurn = static_cast<int>(m_movementArea.m_tileArea.front().dir);
			m_sprite.setRotation(directionToTurn * ROTATION_ANGLE % 360);
			m_currentDirection = (eDirection)directionToTurn;

			m_movementArea.m_tileArea.pop_front();

			if (m_movementArea.m_tileArea.empty())
			{
				m_movingToDestination = false;
				disableMovementGraph();
				m_moving = false;
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

void Ship::renderMovementGraph(sf::RenderWindow & window, const Map & map)
{
	m_movementArea.render(window, map);
}

void Ship::setDeploymentPosition(sf::Vector2i position)
{
	m_currentPosition = position;
}

void Ship::deployAtPosition(sf::Vector2i position, eDirection startingDirection)
{
	m_currentPosition = position;
	m_deployed = true;
	m_sprite.setRotation(startingDirection * ROTATION_ANGLE % 360);
}