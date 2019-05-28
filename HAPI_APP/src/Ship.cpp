#include "Ship.h"

#include "entity.h"
#include "Map.h"
#include "BFS.h"
#include "Textures.h"
#include "GameEventMessenger.h"
#include "Battle.h"

constexpr float MOVEMENT_ANIMATION_TIME(0.35f);
constexpr size_t MOVEMENT_PATH_SIZE{ 32 };


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

std::pair<int, int> Ship::getCurrentPosition() const
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

int Ship::generateMovementPath(const Map & map, std::pair<int, int> destination)
{
	posi start = { m_currentPosition, m_currentDirection };
	posi end = { destination.first, destination.second };
	std::queue<posi> pathToTile = BFS::findPath(map, start, end, m_movementPoints);
	if (pathToTile.empty())
	{
		return 0;
	}
	disableMovementPath();

	int i = 0;
	int queueSize = pathToTile.size();
	for (i; i < queueSize; ++i)
	{
		auto tileScreenPosition = map.getTileScreenPos(pathToTile.front().pair());
		m_movementPath[i].m_sprite->GetTransformComp().SetPosition({
			static_cast<float>(tileScreenPosition.first + DRAW_OFFSET_X * map.getDrawScale()),
			static_cast<float>(tileScreenPosition.second + DRAW_OFFSET_Y * map.getDrawScale()) });
		m_movementPath[i].m_active = true;
		m_movementPath[i].m_position = pathToTile.front().pair();

		pathToTile.pop();
	}

	m_movementPathSize = i - 1;
	return i;
}

void Ship::disableMovementPath()
{
	for (auto& i : m_movementPath)
	{
		i.m_active = false;
	}
}

std::pair<int, int> Ship::getEndOfMovementPath() const
{
	int i = m_movementPath.size() - 1;
	for (i; i > 0; i--)
	{
		if (m_movementPath[i].m_active)
			break;
	}
	return m_movementPath[i].m_position;
}

void Ship::enableAction()
{
	if (!m_isDead)
	{
		m_actionSprite.m_active = true;
	}
}

void Ship::disableAction()
{
	m_actionSprite.m_active = false;
}

bool Ship::move(Map& map, std::pair<int, int> destination)
{
	if (!m_destinationSet)
	{
		posi currentPos = { m_currentPosition, m_currentDirection };
		posi destinationPos = { destination.first, destination.second };
		//TODO: We should not have to go throught the map from the entity to get to the entity movement data!
		std::queue<posi> pathToTile = BFS::findPath(map, currentPos, destinationPos, m_movementPoints);
		if (!pathToTile.empty() && pathToTile.size() <= m_movementPathSize + 1)
		{
			m_pathToTile = pathToTile;
			map.updateShipOnTile({ m_factionName, m_ID }, m_currentPosition, pathToTile.back().pair());
			m_destinationSet = true;
			m_movingToDestination = true;
			m_actionSprite.m_active = false;
			return true;
		}
		else
		{
			disableMovementPath();
			return false;
		}
	}
	disableMovementPath();
	return true;
}

bool Ship::move(Map& map, std::pair<int, int> destination, eDirection endDirection)
{
	if (!m_destinationSet)
	{
		posi currentPos = { m_currentPosition.first, m_currentPosition.second, m_currentDirection };
		posi destinationPos = { destination.first, destination.second };
		std::queue<posi> pathToTile = BFS::findPath(map, currentPos, destinationPos, m_movementPoints);
		if (!pathToTile.empty() && pathToTile.size() <= m_movementPathSize + 1)
		{
			pathToTile.emplace(posi(pathToTile.back().pair(), endDirection));
			m_pathToTile = pathToTile;
			map.updateShipOnTile({ m_factionName, m_ID }, m_currentPosition, pathToTile.back().pair());
			m_destinationSet = true;
			m_movingToDestination = true;
			m_actionSprite.m_active = false;
			return true;
		}
		else
		{
			disableMovementPath();
			return false;
		}
	}
	disableMovementPath();
	return true;
}

void Ship::takeDamage(int damageAmount, FactionName entityFaction)
{
	m_health -= damageAmount;
	int healthPercentage = ((float)m_health / m_maxHealth) * 100;

	if (healthPercentage < 100 && healthPercentage >= 50)
	{
		m_sprite->SetFrameNumber(eShipSpriteFrame::eLowDamage);
		m_sprite->GetTransformComp().SetOriginToCentreOfFrame();
	}
	else if (healthPercentage < 50 && healthPercentage >= 1)
	{
		m_sprite->SetFrameNumber(eShipSpriteFrame::eHighDamage);
		m_sprite->GetTransformComp().SetOriginToCentreOfFrame();
	}
	else
	{
		m_health = 0;
		m_sprite->SetFrameNumber(eShipSpriteFrame::eDead);
		m_sprite->GetTransformComp().SetOriginToCentreOfFrame();
		m_isDead = true;
		m_actionSprite.m_active = false;
		disableMovementPath();
		switch (entityFaction)
		{
		case FactionName::eYellow:
			GameEventMessenger::broadcast(GameEvent::eYellowShipDestroyed);
			break;
		case FactionName::eGreen:
			GameEventMessenger::broadcast(GameEvent::eGreenShipDestroyed);
			break;
		case FactionName::eRed:
			GameEventMessenger::broadcast(GameEvent::eRedShipDestroyed);
			break;
		case FactionName::eBlue:
			GameEventMessenger::broadcast(GameEvent::eBlueShipDestroyed);
			break;
		}
	}
}

void Ship::fireWeapon()
{
	m_weaponFired = true;
	m_actionSprite.m_active = false;
}

void Ship::setDestination()
{
	m_destinationSet = true;
}

void Ship::onNewTurn()
{
	//m_movedToDestination = false;
	m_weaponFired = false;
	m_destinationSet = false;
	m_movingToDestination = false;
}

void Ship::disableMovementPathNode(std::pair<int, int> position, const Map & map)
{
	for (auto iter = m_movementPath.begin(); iter != m_movementPath.end(); ++iter)
	{
		auto i = map.getMouseClickCoord({ iter->m_sprite->GetTransformComp().GetPosition().x, iter->m_sprite->GetTransformComp().GetPosition().y });
		if (i == position)
		{
			iter->m_active = false;
		}
	}
}

void Ship::handleRotation()
{
	int rotationAngle = 60;
	int directionToTurn = static_cast<int>(m_pathToTile.front().dir);
	m_sprite->GetTransformComp().SetRotation(DEGREES_TO_RADIANS(directionToTurn*rotationAngle % 360));
	m_currentDirection = (eDirection)directionToTurn;
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
	m_pathToTile(),
	m_movementTimer(MOVEMENT_ANIMATION_TIME),
	m_movementPathSize(0),
	m_currentDirection(),
	m_weaponFired(false),
	m_isDead(false),
	m_actionSprite(factionName, 2.f, 2.f),
	m_movingToDestination(false),
	m_destinationSet(false),
	m_maxHealth(0),
	m_health(0),
	m_damage(0),
	m_range(0),
	m_movementPoints(0),
	m_sprite(),
	m_deployed(false),
	m_movementPath()
{
	//Initialize Movement Path
	m_movementPath.reserve(size_t(MOVEMENT_PATH_SIZE));
	for (int i = 0; i < MOVEMENT_PATH_SIZE; ++i)
	{
		m_movementPath.emplace_back(Textures::m_spawnHex, 0.5f, 0.5f);
	}

	//Initialize Ship
	switch (shipType)
	{
	case eShipType::eFrigate:
		m_movementPoints = 5;
		m_health = 5;
		m_range = 5;
		m_damage = 5;
		break;
	case eShipType::eTurtle:
		m_movementPoints = 8;
		m_health = 20;
		m_range = 1;
		m_damage = 2;
		break;
	case eShipType::eFire:
		m_health = 8;
		m_movementPoints = 10;
		m_range = 2;
		m_damage = 6;
		break;
	case eShipType::eSniper:
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
			m_sprite = std::unique_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_yellowShipSideCannons));
			break;
		case eShipType::eTurtle:
			m_sprite = std::unique_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_yellowShipBomb));
			break;
		case eShipType::eFire:
			m_sprite = std::unique_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_yellowShipMelee));
			break;
		case eShipType::eSniper:
			m_sprite = std::unique_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_yellowShipSnipe));
			break;
		}
		break;

	case FactionName::eBlue:
		switch (shipType)
		{
		case eShipType::eFrigate:
			m_sprite = std::unique_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_blueShipSideCannons));
			break;
		case eShipType::eTurtle:
			m_sprite = std::unique_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_blueShipBomb));
			break;
		case eShipType::eFire:
			m_sprite = std::unique_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_blueShipMelee));
			break;
		case eShipType::eSniper:
			m_sprite = std::unique_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_blueShipSnipe));
			break;
		}
		break;
	case FactionName::eRed:
		switch (shipType)
		{
		case eShipType::eFrigate:
			m_sprite = std::unique_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_redShipSideCannons));
			break;
		case eShipType::eTurtle:
			m_sprite = std::unique_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_redShipBomb));
			break;
		case eShipType::eFire:
			m_sprite = std::unique_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_redShipMelee));
			break;
		case eShipType::eSniper:
			m_sprite = std::unique_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_redShipSnipe));
			break;
		default:
			break;
		}
		break;
	case FactionName::eGreen:
		switch (shipType)
		{
		case eShipType::eFrigate:
			m_sprite = std::unique_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_greenShipSideCannons));
			break;
		case eShipType::eTurtle:
			m_sprite = std::unique_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_greenShipBomb));
			break;
		case eShipType::eFire:
			m_sprite = std::unique_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_greenShipMelee));
			break;
		case eShipType::eSniper:
			m_sprite = std::unique_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_greenShipSnipe));
			break;
		}
		break;
	}

	m_sprite->SetFrameNumber(eShipSpriteFrame::eMaxHealth);
	m_sprite->GetTransformComp().SetOriginToCentreOfFrame();
	m_sprite->GetTransformComp().SetScaling({ 1, 1 }); // Might not need
}

Ship::Ship(Ship & orig)
	: m_factionName(orig.m_factionName),
	m_shipType(orig.m_shipType),
	m_ID(orig.m_ID),
	m_currentPosition(),
	m_pathToTile(),
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
	m_sprite(),
	m_deployed(false),
	m_movementPath()
{
	m_sprite.swap(orig.m_sprite);
}

void Ship::update(float deltaTime, const Map & map)
{
	if (!m_pathToTile.empty())
	{
		m_movementTimer.update(deltaTime);
		if (m_movementTimer.isExpired())
		{
			m_movementTimer.reset();
			m_currentPosition = m_pathToTile.front().pair();
			disableMovementPathNode(m_currentPosition, map);

			handleRotation();
			m_pathToTile.pop();

			if (m_pathToTile.empty())
			{
				m_movingToDestination = false;
				disableMovementPath();
			}
		}
	}
}

void Ship::render(const Map & map) const
{
	for (const auto& i : m_movementPath)
	{
		if (i.m_active)
		{
			const std::pair<int, int> tileTransform = map.getTileScreenPos(i.m_position);
			float scale = map.getDrawScale();

			i.m_sprite->GetTransformComp().SetPosition({
				static_cast<float>(tileTransform.first + DRAW_OFFSET_X * scale),
				static_cast<float>(tileTransform.second + DRAW_OFFSET_Y * scale) });
			i.m_sprite->GetTransformComp().SetScaling({ 0.5f, 0.5f });

			i.m_sprite->Render(SCREEN_SURFACE);
		}
	}

	//Set sprite position to current position
	const std::pair<int, int> tileTransform = map.getTileScreenPos(m_currentPosition);
	float scale = map.getDrawScale();

	m_sprite->GetTransformComp().SetPosition({
		static_cast<float>(tileTransform.first + DRAW_OFFSET_X * scale),
		static_cast<float>(tileTransform.second + DRAW_OFFSET_Y * scale) });
	m_sprite->GetTransformComp().SetScaling({ scale / 2, scale / 2 });

	m_sprite->Render(SCREEN_SURFACE);
	m_actionSprite.render(map, m_currentPosition);
}

void Ship::setDeploymentPosition(std::pair<int, int> position)
{
	m_currentPosition = position;
}

void Ship::deployAtPosition(std::pair<int, int> position, eDirection startingDirection)
{
	m_currentPosition = position;
	m_deployed = true;
	m_sprite->GetTransformComp().SetRotation(DEGREES_TO_RADIANS(startingDirection * 60 % 360));
}
