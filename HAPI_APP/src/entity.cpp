#include "entity.h"
#include "Map.h"
#include "BFS.h"
#include "Textures.h"
#include "GameEventMessenger.h"
#include "Battle.h"

constexpr float MOVEMENT_ANIMATION_TIME(0.35f);
constexpr size_t MOVEMENT_PATH_SIZE{ 32 };
constexpr size_t WEAPON_HIGHLIGHT_SIZE{ 200 };
constexpr float DRAW_ENTITY_OFFSET_X{ 16 };
constexpr float DRAW_ENTITY_OFFSET_Y{ 32 };


FactionName Ship::getFactionName() const
{
	return m_factionName;
}

eDirection Ship::getCurrentDirection() const
{
	return m_currentDirection;
}

eWeaponType Ship::getWeaponType() const
{
	return m_weaponType;
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
	std::queue<posi> pathToTile = BFS::findPath(map, start, end, source.m_shipOnTile->getMovementPoints());
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
		std::queue<posi> pathToTile = BFS::findPath(map, currentPos, destination, map.getTile(currentPos)->m_shipOnTile->getMovementPoints());
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
		std::queue<posi> pathToTile = BFS::findPath(map, currentPos, destination, map.getTile(currentPos)->m_shipOnTile->getMovementPoints());
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

void Ship::handleRotation()
{
	int rotationAngle = 60;
	int directionToTurn = static_cast<int>(m_pathToTile.front().dir);
	m_sprite->GetTransformComp().SetRotation(DEGREES_TO_RADIANS(directionToTurn*rotationAngle % 360));
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

Ship::Ship(FactionName factionName, ShipType shipType, int health, int damage, int range, eWeaponType weaponType)
	: m_factionName(factionName),
	m_shipType(shipType),
	m_currentPosition(),
	m_pathToTile(),
	m_movementTimer(MOVEMENT_ANIMATION_TIME),
	m_movementPath(),
	m_movementPathSize(0),
	m_currentDirection(),
	m_weaponFired(false),
	m_isDead(false),
	m_actionSprite(factionName),
	m_movingToDestination(false),
	m_destinationSet(false),
	m_health(0),
	m_damage(0),
	m_range(0),
	m_movementPoints(0),
	m_weaponType(),
	m_sprite(),
	m_deployed(false)
{
	switch (shipType)
	{
	case ShipType::eFrigate:
		m_movementPoints = 5;
		m_health = 5;
		m_range = 5;
		m_damage = 5;
		m_weaponType = eWeaponType::eSideCannons;
		break;
	case ShipType::eTurtle:
		m_movementPoints = 8;
		m_health = 20;
		m_range = 1;
		m_damage = 2;
		m_weaponType = eWeaponType::eShotgun;
		break;
	case ShipType::eFire:
		m_health = 8;
		m_movementPoints = 10;
		m_range = 2;
		m_damage = 6;
		m_weaponType = eWeaponType::eFlamethrower;
		break;
	case ShipType::eSniper:
		m_health = 8;
		m_movementPoints = 6;
		m_range = 10;
		m_damage = 4;
		m_weaponType = eWeaponType::eStraightShot;
		break;
	}

	switch (factionName)
	{
	case FactionName::eYellow:
		switch (shipType)
		{
		case ShipType::eFrigate:
			m_sprite = std::unique_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_yellowShipSideCannons));
			break;
		case ShipType::eTurtle:
			m_sprite = std::unique_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_yellowShipBomb));
			break;
		case ShipType::eFire:
			m_sprite = std::unique_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_yellowShipMelee));
			break;
		case ShipType::eSniper:
			m_sprite = std::unique_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_yellowShipSnipe));
			break;
		}
		break;

	case FactionName::eBlue:
		switch (shipType)
		{
		case ShipType::eFrigate:
			m_sprite = std::unique_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_blueShipSideCannons));
			break;
		case ShipType::eTurtle:
			m_sprite = std::unique_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_blueShipBomb));
			break;
		case ShipType::eFire:
			m_sprite = std::unique_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_blueShipMelee));
			break;
		case ShipType::eSniper:
			m_sprite = std::unique_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_blueShipSnipe));
			break;
		}
		break;
	case FactionName::eRed:
		switch (shipType)
		{
		case ShipType::eFrigate:
			m_sprite = std::unique_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_redShipSideCannons));
			break;
		case ShipType::eTurtle:
			m_sprite = std::unique_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_redShipBomb));
			break;
		case ShipType::eFire:
			m_sprite = std::unique_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_redShipMelee));
			break;
		case ShipType::eSniper:
			m_sprite = std::unique_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_redShipSnipe));
			break;
		default:
			break;
		}
		break;
	case FactionName::eGreen:
		switch (shipType)
		{
		case ShipType::eFrigate:
			m_sprite = std::unique_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_greenShipSideCannons));
			break;
		case ShipType::eTurtle:
			m_sprite = std::unique_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_greenShipBomb));
			break;
		case ShipType::eFire:
			m_sprite = std::unique_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_greenShipMelee));
			break;
		case ShipType::eSniper:
			m_sprite = std::unique_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_greenShipSnipe));
			break;
		}
		break;
	}

	m_sprite->SetFrameNumber(eShipSpriteFrame::eMaxHealth);
	m_sprite->GetTransformComp().SetOriginToCentreOfFrame();
	m_sprite->GetTransformComp().SetScaling({ 1, 1 }); // Might not need
	
	GameEventMessenger::getInstance().subscribe(std::bind(&Ship::onNewTurn, this), "Ship", GameEvent::eNewTurn);
}

Ship::~Ship()
{
	GameEventMessenger::getInstance().unsubscribe("Ship", GameEvent::eNewTurn);
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
			m_movementPath.eraseNode(m_currentPosition, map);

			handleRotation();
			m_pathToTile.pop();

			if (m_pathToTile.empty())
			{
				m_movingToDestination = false;
				clearMovementPath();
			}
		}
	}
}

void Ship::render(const Map & map) const
{
	m_movementPath.render(map);

	//Set sprite position to current position
	const std::pair<int, int> tileTransform = map.getTileScreenPos(m_currentPosition);
	float scale = map.getDrawScale();

	m_sprite->GetTransformComp().SetPosition({
		static_cast<float>(tileTransform.first + DRAW_ENTITY_OFFSET_X * scale),
		static_cast<float>(tileTransform.second + DRAW_ENTITY_OFFSET_Y * scale) });
	m_sprite->GetTransformComp().SetScaling({ scale / 2, scale / 2 });

	m_sprite->Render(SCREEN_SURFACE);
	m_actionSprite.render(map, m_currentPosition);
}

void Ship::setDeploymentPosition(std::pair<int, int> position, const Battle & battle)
{
	assert(battle.getCurrentPhase() == BattlePhase::Deployment);
	m_currentPosition = position;
}

void Ship::deployAtPosition(std::pair<int, int> position, Battle & battle, eDirection startingDirection)
{
	assert(battle.getCurrentPhase() == BattlePhase::Deployment);
	m_currentPosition = position;
	m_deployed = true;
	m_sprite->GetTransformComp().SetRotation(DEGREES_TO_RADIANS(startingDirection * 60 % 360));
}

//BATTLE PLAYER
Player::Player(FactionName name, ePlayerType playerType)
	: m_ships(),
	m_factionName(name),
	m_playerType(playerType),
	m_eliminated(false),
	m_shipToDeploy(nullptr),
	m_spawnArea()
{
	//for (int i = 0; i < m_spawnArea.size(); ++i)
	//{
	//	std::unique_ptr<Sprite> sprite;
	//	switch (playerToDeploy.m_factionName)
	//	{
	//	case eYellow:
	//		sprite = HAPI_Sprites.MakeSprite(Textures::m_yellowSpawnHex);
	//		break;
	//	case eBlue:
	//		sprite = HAPI_Sprites.MakeSprite(Textures::m_blueSpawnHex);
	//		break;
	//	case eGreen:
	//		sprite = HAPI_Sprites.MakeSprite(Textures::m_greenSpawnHex);
	//		break;
	//	case eRed:
	//		sprite = HAPI_Sprites.MakeSprite(Textures::m_redSpawnHex);
	//		break;
	//	};
	//	auto screenPosition = map.getTileScreenPos(m_spawnArea[i]->m_tileCoordinate);
	//	sprite->GetTransformComp().SetPosition({
	//		(float)screenPosition.first + DRAW_ENTITY_OFFSET_X * map.getDrawScale(),
	//		(float)screenPosition.second + DRAW_ENTITY_OFFSET_Y * map.getDrawScale() });
	//	sprite->GetTransformComp().SetOriginToCentreOfFrame();
	//	sprite->GetTransformComp().SetScaling({ 2.f, 2.f });
	//	m_spawnSprites.push_back(std::move(sprite));
	//}
}

void Player::render(const Map & map) const
{
	for (const auto& spawnArea : m_spawnArea)
	{
		spawnArea.render(map);
	}

	for (const auto& ship : m_ships)
	{
		if (ship->isDeployed())
		{
			ship->render(map);
		}
	}

	m_shipToDeploy->render(map);
}

void Player::createSpawnArea(Map & map)
{
	std::vector<const Tile*> tileRadius = map.cGetTileRadius(map.getRandomSpawnPosition(), 3, true, true);
	m_spawnArea.reserve(tileRadius.size());
	for (const auto& tile : tileRadius)
	{
		m_spawnArea.emplace_back(m_factionName, tile->m_tileCoordinate, map);
	}
}

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

Player::SpawnNode::SpawnNode(FactionName factionName, std::pair<int, int> position, const Map & map)
	: m_position(position),
	m_sprite()
{
	switch (factionName)
	{
	case eYellow:
		m_sprite = HAPI_Sprites.MakeSprite(Textures::m_yellowSpawnHex);
		break;
	case eBlue:
		m_sprite = HAPI_Sprites.MakeSprite(Textures::m_blueSpawnHex);
		break;
	case eGreen:
		m_sprite = HAPI_Sprites.MakeSprite(Textures::m_greenSpawnHex);
		break;
	case eRed:
		m_sprite = HAPI_Sprites.MakeSprite(Textures::m_redSpawnHex);
		break;
	};
	
	auto screenPosition = map.getTileScreenPos(position);
	m_sprite->GetTransformComp().SetPosition({
		(float)screenPosition.first + DRAW_ENTITY_OFFSET_X * map.getDrawScale(),
		(float)screenPosition.second + DRAW_ENTITY_OFFSET_Y * map.getDrawScale() });
	m_sprite->GetTransformComp().SetOriginToCentreOfFrame();
	m_sprite->GetTransformComp().SetScaling({ 2.f, 2.f });
}

void Player::SpawnNode::render(const Map & map) const
{
	auto screenPosition = map.getTileScreenPos(m_position);
	m_sprite->GetTransformComp().SetPosition({
	(float)screenPosition.first + DRAW_ENTITY_OFFSET_X * map.getDrawScale(),
	(float)screenPosition.second + DRAW_ENTITY_OFFSET_Y * map.getDrawScale() });
	m_sprite->Render(SCREEN_SURFACE);
}