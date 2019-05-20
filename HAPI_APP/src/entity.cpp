#include "entity.h"
#include "Map.h"
#include "BFS.h"
#include "Textures.h"
#include "GameEventMessenger.h"

constexpr float MOVEMENT_ANIMATION_TIME(0.35f);
constexpr size_t MOVEMENT_PATH_SIZE{ 32 };
constexpr size_t WEAPON_HIGHLIGHT_SIZE{ 200 };
constexpr float DRAW_ENTITY_OFFSET_X{ 16 };
constexpr float DRAW_ENTITY_OFFSET_Y{ 32 };
constexpr int UPGRADE_POINTS = 2;

struct PathNode;

std::vector<ShipGlobalProperties> assignEntities(FactionName name)
{
	std::vector<ShipGlobalProperties> entities;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			ShipGlobalProperties newEntity(name, (ShipType)(i));
			entities.push_back(newEntity);
		}
	}
	assert(!entities.empty());
	return entities;
}


ShipGlobalProperties & Ship::getProperties()
{
	return m_entityProperties;
}

FactionName Ship::getFactionName() const
{
	return m_factionName;
}

eDirection Ship::getCurrentDirection() const
{
	return m_currentDirection;
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

//MOVEMENT PATH NODE
Ship::MovementPath::PathNode::PathNode()
	: sprite(std::make_unique<Sprite>(Textures::m_spawnHex)),
	activate(false)
{
	sprite->GetTransformComp().SetOriginToCentreOfFrame();
	sprite->GetTransformComp().SetScaling({ 0.5f, 0.5f });
}

//
//MOVEMENT PATH
//
Ship::MovementPath::MovementPath()
	: m_movementPath()	
{
	m_movementPath.reserve(size_t(MOVEMENT_PATH_SIZE));
	for (int i = 0; i < MOVEMENT_PATH_SIZE; ++i)
	{
		m_movementPath.push_back({});
	}
}

void Ship::MovementPath::render(const Map& map) const
{
	for (const auto& i : m_movementPath)
	{
		if (i.activate)
		{
			const std::pair<int, int> tileTransform = map.getTileScreenPos(i.m_position);
			float scale = map.getDrawScale();

			i.sprite->GetTransformComp().SetPosition({
				static_cast<float>(tileTransform.first + DRAW_ENTITY_OFFSET_X * scale),
				static_cast<float>(tileTransform.second + DRAW_ENTITY_OFFSET_Y * scale) });
			i.sprite->GetTransformComp().SetScaling({ 0.5f, 0.5f });

			i.sprite->Render(SCREEN_SURFACE);
		}	
	}
}

int Ship::MovementPath::generatePath(const Map& map, const Tile& source, const Tile& destination)
{
	posi start = { source.m_tileCoordinate.first, source.m_tileCoordinate.second, source.m_shipOnTile->m_currentDirection };
	posi end = { destination.m_tileCoordinate.first, destination.m_tileCoordinate.second };
	std::queue<posi> pathToTile = BFS::findPath(map, start, end, source.m_shipOnTile->m_entityProperties.m_movementPoints);
	if (pathToTile.empty())
	{
		return 0;
	}
	clearPath();
	//float movementPointsUsed = 0;
	if (!source.m_shipOnTile)
		return 0;
	//int prevDir = source.m_entityOnTile->m_battleProperties.m_currentDirection;
	std::pair<int, int> prevPos = source.m_tileCoordinate;
	//float windStrength = map.getWindStrength();
	//int windDirection = static_cast<int>(map.getWindDirection());
	//Don't interact with path from source.
	int i = 0;
	int queueSize = pathToTile.size();
	for (i ; i < queueSize; ++i)
	{
		/*
		//If moved from prev position handle forward cost
		if (pathToTile.front().pair() != prevPos)
		{
			movementPointsUsed += 1;
			if (prevDir == windDirection)
			{
				movementPointsUsed -= windStrength;
			}
		}
		//Turning cost handling
		int pathDir = pathToTile.front().dir;
		movementPointsUsed += static_cast<float>(getDirectionCost(prevDir, pathDir));
		prevDir = pathDir;

		*/
		//if ((static_cast<float>(source.m_entityOnTile->m_entityProperties.m_movementPoints) - movementPointsUsed) >= 0)
		{
			auto tileScreenPosition = map.getTileScreenPos(pathToTile.front().pair());
			m_movementPath[i].sprite->GetTransformComp().SetPosition({
				static_cast<float>(tileScreenPosition.first + DRAW_OFFSET_X * map.getDrawScale()),
				static_cast<float>(tileScreenPosition.second + DRAW_OFFSET_Y * map.getDrawScale()) });
			m_movementPath[i].activate = true;
			m_movementPath[i].m_position = pathToTile.front().pair();
		}
		//else
		//{
		//	source.m_entityOnTile->m_battleProperties.m_movementPathSize = i - 1;
		//	return i;
		//}
		pathToTile.pop();
	}
	source.m_shipOnTile->m_movementPathSize = i - 1;
	return i;
}

void Ship::MovementPath::eraseNode(std::pair<int, int> position, const Map& map)
{
	for (auto iter = m_movementPath.begin(); iter != m_movementPath.end(); ++iter)
	{
		auto i = map.getMouseClickCoord({ iter->sprite->GetTransformComp().GetPosition().x, iter->sprite->GetTransformComp().GetPosition().y });
		if (i == position)
		{
			iter->activate = false;
		}
	}
}

void Ship::MovementPath::clearPath()
{
	for (auto& i : m_movementPath)
	{
		i.activate = false;
	}
}

std::pair<int, int> Ship::MovementPath::getFinalNode() const
{
	int i = m_movementPath.size() - 1;
	for (i; i > 0; i--)
	{
		if (m_movementPath[i].activate)
			break;
	}
	return m_movementPath[i].m_position;
}

std::vector<posi> Ship::generateMovementArea(const Map& map, float movement) const
{
	posi position = posi(m_currentPosition, m_currentDirection);
	return BFS::findArea(map, position, movement);
}

int Ship::generateMovementGraph(const Map & map, const Tile & source, const Tile & destination)
{
	return m_movementPath.generatePath(map, source, destination);
}

void Ship::clearMovementPath()
{
	m_movementPath.clearPath();
}

std::pair<int, int> Ship::getEndOfPath()
{
	return m_movementPath.getFinalNode();
}

void Ship::enableAction()
{
	if (!m_isDead)
	{
		m_actionSprite.active = true;
	}
}

void Ship::disableAction()
{
	m_actionSprite.active = false;
}

bool Ship::moveEntity(Map& map, const Tile& tile)
{
	if (!m_destinationSet)
	{
		posi currentPos = { m_currentPosition, m_currentDirection };
		posi destination = { tile.m_tileCoordinate.first, tile.m_tileCoordinate.second };
		//TODO: We should not have to go throught the map from the entity to get to the entity movement data!
		std::queue<posi> pathToTile = BFS::findPath(map, currentPos, destination, map.getTile(currentPos)->m_shipOnTile->m_entityProperties.m_movementPoints);
		if (!pathToTile.empty() && pathToTile.size() <= m_movementPathSize + 1)
		{
			m_pathToTile = pathToTile;
			map.updateTileNewShipPosition(m_currentPosition, pathToTile.back().pair());
			m_destinationSet = true;
			m_movingToDestination = true;
			m_actionSprite.active = false;
			return true;
		}
		else
		{
			clearMovementPath();
			return false;
		}
	}
	clearMovementPath();
	return true;
}

bool Ship::moveEntity(Map& map, const Tile& tile, eDirection endDirection)
{
	if (!m_destinationSet)
	{
		posi currentPos = { m_currentPosition.first, m_currentPosition.second, m_currentDirection };
		posi destination = { tile.m_tileCoordinate.first, tile.m_tileCoordinate.second };
		//TODO: We should not have to go throught the map from the entity to get to the entity movement data!
		std::queue<posi> pathToTile = BFS::findPath(map, currentPos, destination, map.getTile(currentPos)->m_shipOnTile->m_entityProperties.m_movementPoints);
		if (!pathToTile.empty() && pathToTile.size() <= m_movementPathSize + 1)
		{
			pathToTile.emplace(posi(pathToTile.back().pair(), endDirection));
			m_pathToTile = pathToTile;
			map.updateTileNewShipPosition(m_currentPosition, pathToTile.back().pair());
			m_destinationSet = true;
			m_movingToDestination = true;
			m_actionSprite.active = false;
			return true;
		}
		else
		{
			clearMovementPath();
			return false;
		}
	}
	clearMovementPath();
	return true;
}
 
void Ship::takeDamage(ShipGlobalProperties & entityProperties, int damageAmount, FactionName entityFaction)
{
	entityProperties.m_currentHealth -= damageAmount;
	int currentHealth = entityProperties.m_currentHealth;
	int maxHealth = entityProperties.m_healthMax;
	auto& entitySprite = entityProperties.m_sprite;

	int healthPercentage = ((float)currentHealth / maxHealth) * 100;
	
	if (healthPercentage < 100 && healthPercentage >= 50)
	{
		entitySprite->SetFrameNumber(eShipSpriteFrame::eLowDamage);
		entitySprite->GetTransformComp().SetOriginToCentreOfFrame();
	}
	else if (healthPercentage < 50 && healthPercentage >= 1)
	{
		entitySprite->SetFrameNumber(eShipSpriteFrame::eHighDamage);
		entitySprite->GetTransformComp().SetOriginToCentreOfFrame();
	}
	else
	{
		currentHealth = 0;
		entitySprite->SetFrameNumber(eShipSpriteFrame::eDead);
		entitySprite->GetTransformComp().SetOriginToCentreOfFrame();
		m_isDead = true;
		m_actionSprite.active = false;
		m_movementPath.clearPath();
		switch (entityFaction)
		{
		case FactionName::eYellow :
			GameEventMessenger::broadcast(GameEvent::eYellowShipDestroyed);
			break;
		case FactionName::eGreen :
			GameEventMessenger::broadcast(GameEvent::eGreenShipDestroyed);
			break;
		case FactionName::eRed :
			GameEventMessenger::broadcast(GameEvent::eRedShipDestroyed);
			break;
		case FactionName::eBlue :
			GameEventMessenger::broadcast(GameEvent::eBlueShipDestroyed);
			break;
		}
	}
}

void Ship::fireWeapon()
{
	m_weaponFired = true;
	m_actionSprite.active = false;
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

void Ship::handleRotation(ShipGlobalProperties& entityProperties)
{
	int rotationAngle = 60;
	int directionToTurn = static_cast<int>(m_pathToTile.front().dir);
	entityProperties.m_sprite->GetTransformComp().SetRotation(
		DEGREES_TO_RADIANS(directionToTurn*rotationAngle % 360));
	m_currentDirection = (eDirection)directionToTurn;
}

unsigned int Ship::MovementPath::getDirectionCost(int currentDirection, int newDirection)
{
	unsigned int diff = std::abs(newDirection - currentDirection);
	if (diff == 0)
	{
		return 0;
	}
	//number of direction % difference between the new and old directions
	return (static_cast<int>(eDirection::Max) % diff) + 1;
}



Ship::Ship(std::pair<int, int> startingPosition, const ShipGlobalProperties& entityProperties, Map& map, FactionName factionName, eDirection startingDirection)
	: m_factionName(factionName),
	m_entityProperties(entityProperties),
	m_currentPosition(startingPosition),
	m_pathToTile(),
	m_movementTimer(MOVEMENT_ANIMATION_TIME),
	m_movementPath(),
	m_movementPathSize(0),
	m_currentDirection(startingDirection),
	m_weaponFired(false),
	m_isDead(false),
	m_actionSprite(factionName),
	m_movingToDestination(false),
	m_destinationSet(false)

	int m_health;
int m_damage;
int m_range;
eWeaponType m_weaponType;

{
	m_entityProperties.m_sprite->GetTransformComp().SetRotation(DEGREES_TO_RADIANS(startingDirection * 60 % 360));
	map.assignTileToShip(*this);
	GameEventMessenger::getInstance().subscribe(std::bind(&Ship::onNewTurn, this), "Ship", GameEvent::eNewTurn);
}

Ship::~Ship()
{
	GameEventMessenger::getInstance().unsubscribe("Ship", GameEvent::eNewTurn);
}

void Ship::update(float deltaTime, const Map & map, ShipGlobalProperties& entityProperties)
{	
	if (!m_pathToTile.empty())
	{
		m_movementTimer.update(deltaTime);
		if (m_movementTimer.isExpired())
		{
			m_movementTimer.reset();
			m_currentPosition = m_pathToTile.front().pair();
			m_movementPath.eraseNode(m_currentPosition, map);

			handleRotation(entityProperties);
			m_pathToTile.pop();

			if (m_pathToTile.empty())
			{
				m_movingToDestination = false;
				clearMovementPath();
			}
		}
	}
}

void Ship::render(std::shared_ptr<HAPISPACE::Sprite>& sprite, const Map & map)
{
	//Set sprite position to current position
	const std::pair<int, int> tileTransform = map.getTileScreenPos(m_currentPosition);
	float scale = map.getDrawScale();

	sprite->GetTransformComp().SetPosition({
		static_cast<float>(tileTransform.first + DRAW_ENTITY_OFFSET_X * scale),
		static_cast<float>(tileTransform.second + DRAW_ENTITY_OFFSET_Y * scale) });
	sprite->GetTransformComp().SetScaling({ scale / 2, scale / 2 });

	sprite->Render(SCREEN_SURFACE);
	m_actionSprite.render(map, m_currentPosition);
}

void Ship::renderPath(const Map & map)
{
	m_movementPath.render(map);
}

//BATTLE PLAYER
BattlePlayer::BattlePlayer(FactionName name, std::pair<int, int> spawnPosition, ePlayerType playerType)
	: m_entities(),
	m_factionName(name),
	m_playerType(playerType),
	m_spawnPosition(spawnPosition),
	m_eliminated(false)
{}

Ship::ActionSprite::ActionSprite(FactionName factionName)
	: sprite(),
	active(false)
{
	switch (factionName)
	{
	case FactionName::eYellow :
		sprite = HAPI_Sprites.MakeSprite(Textures::m_yellowSpawnHex);
		break;
	case FactionName::eBlue :
		sprite = HAPI_Sprites.MakeSprite(Textures::m_blueSpawnHex);
		break;
	case FactionName::eGreen :
		sprite = HAPI_Sprites.MakeSprite(Textures::m_greenSpawnHex);
		break;
	case FactionName::eRed :
		sprite = HAPI_Sprites.MakeSprite(Textures::m_redSpawnHex);
		break;
	}

	sprite->GetTransformComp().SetOriginToCentreOfFrame();
	sprite->GetTransformComp().SetScaling({ 2.f, 2.f });
}

void Ship::ActionSprite::render(const Map& map, std::pair<int, int> currentEntityPosition) const
{
	if (active)
	{
		auto screenPosition = map.getTileScreenPos(currentEntityPosition);
		sprite->GetTransformComp().SetPosition({
		(float)screenPosition.first + DRAW_ENTITY_OFFSET_X * map.getDrawScale() ,
		(float)screenPosition.second + DRAW_ENTITY_OFFSET_Y * map.getDrawScale() });
		sprite->Render(SCREEN_SURFACE);
	}
}

Player::Player(FactionName name, ePlayerType playerType)
	: m_entities(assignEntities(name)),
	m_selectedEntities(),
	m_factionName(name),
	m_type(playerType)
{}