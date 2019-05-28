#include "Battle.h"
#include "Utilities/MapParser.h"
#include "GameEventMessenger.h"
#include "AI.h"

using namespace HAPISPACE;

Battle::Particle::Particle(float lifespan, std::shared_ptr<HAPISPACE::SpriteSheet> texture, float scale) :
	m_position(),
	m_lifeSpan(lifespan),
	m_particle(HAPI_Sprites.MakeSprite(texture)),
	m_frameNum(0),
	m_isEmitting(false),
	m_scale(scale)
{
	m_particle->SetFrameNumber(m_frameNum);
}

void Battle::Particle::setPosition(std::pair<int, int> position)
{
	m_position = position;
}

void Battle::Particle::update(float deltaTime, const Map& map)
{
	if (m_isEmitting)
	{
		const std::pair<int, int> tileTransform = map.getTileScreenPos(m_position);
		m_particle->GetTransformComp().SetPosition({
			tileTransform.first + DRAW_OFFSET_X * map.getDrawScale(),
			tileTransform.second + DRAW_OFFSET_Y * map.getDrawScale() });

		m_lifeSpan.update(deltaTime);

		if (m_lifeSpan.isExpired())
		{
			m_particle->SetFrameNumber(m_frameNum);

			m_lifeSpan.reset();
			++m_frameNum;
		}

		if (m_frameNum >= m_particle->GetNumFrames())
		{
			m_isEmitting = false;
			m_frameNum = 0;
		}
	}
}

void Battle::Particle::render()const
{
	if (m_isEmitting)
	{
		m_particle->GetTransformComp().SetOriginToCentreOfFrame();
		m_particle->GetTransformComp().SetScaling(m_scale);
		m_particle->Render(SCREEN_SURFACE);
	}
}

void Battle::Particle::orient(eDirection entityDir)
{
	eDirection direction = eNorth;
	switch (entityDir)
	{
	case eNorth:
		direction = eSouth;
		break;
	case eNorthEast:
		direction = eSouthWest;
		break;
	case eSouthEast:
		direction = eNorthWest;
		break;
	case eSouth:
		direction = eNorth;
		break;
	case eSouthWest:
		direction = eNorthEast;
		break;
	case eNorthWest:
		direction = eSouthEast;
		break;
	}
	m_particle->GetTransformComp().SetRotation(DEGREES_TO_RADIANS(static_cast<int>(direction) * 60 % 360));
}

void Battle::updateWindDirection()
{
	int turnLeft = rand() % 2;
	eDirection windDirection = eNorth;
	if (turnLeft)
	{
		switch (m_map.getWindDirection())
		{
		case eNorth: windDirection = eNorthWest;
			break;
		case eNorthEast: windDirection = eNorth;
			break;
		case eSouthEast: windDirection = eNorthEast;
			break;
		case eSouth: windDirection = eSouthEast;
			break;
		case eSouthWest: windDirection = eSouth;
			break;
		case eNorthWest: windDirection = eSouthWest;
			break;
		}
	}
	else
	{
		switch (m_map.getWindDirection())
		{
		case eNorth: windDirection = eNorthEast;
			break;
		case eNorthEast: windDirection = eSouthEast;
			break;
		case eSouthEast: windDirection = eSouth;
			break;
		case eSouth: windDirection = eSouthWest;
			break;
		case eSouthWest: windDirection = eNorthWest;
			break;
		case eNorthWest: windDirection = eNorth;
			break;
		}
	}

	m_map.setWindDirection(windDirection);
}

void Battle::handleAIMovementPhaseTimer(float deltaTime)
{
	m_timeUntilAITurn.update(deltaTime);
	if (m_timeUntilAITurn.isExpired())
	{
		m_timeUntilAITurn.reset();
		m_timeUntilAITurn.setActive(false);
		m_timeBetweenAIUnits.setActive(true);
	}

	m_timeBetweenAIUnits.update(deltaTime);
	if (m_timeBetweenAIUnits.isExpired())
	{
		int i = 0;
		for (auto& ship : m_factions[m_currentFactionTurn]->m_ships)
		{
			if (!ship.isDead() &&
				!ship.isDestinationSet())
			{
				AI::handleMovementPhase(*this, m_map, m_factions[m_currentFactionTurn], i);
				m_timeBetweenAIUnits.reset();
				return;
			}
			i++;
		}
		m_timeBetweenAIUnits.setActive(false);
		m_timeBetweenAIUnits.reset();
	}
}

void Battle::handleAIAttackPhaseTimer(float deltaTime)
{
	m_timeUntilAITurn.update(deltaTime);
	if (m_timeUntilAITurn.isExpired())
	{
		m_timeUntilAITurn.reset();
		m_timeUntilAITurn.setActive(false);
		m_timeBetweenAIUnits.setActive(true);
	}

	m_timeBetweenAIUnits.update(deltaTime);
	if (m_timeBetweenAIUnits.isExpired())
	{
		int i = 0;
		for (auto& ship : m_factions[m_currentFactionTurn]->m_ships)
		{
			if (!ship.isDead() && !ship.isWeaponFired())
			{
				AI::handleShootingPhase(*this, m_map, m_factions[m_currentFactionTurn], i);
				m_timeBetweenAIUnits.reset();
				return;
			}
			i++;
		}
		m_timeBetweenAIUnits.setActive(false);
		m_timeBetweenAIUnits.reset();
	}
}

Battle::Battle(std::array<std::unique_ptr<Faction>, static_cast<size_t>(FactionName::MAX)>& players)
	: m_factions(players),
	m_currentFactionTurn(0),
	m_map(),
	m_currentBattlePhase(BattlePhase::Deployment),
	m_battleUI(*this),
	m_explosionParticles(),
	m_fireParticles(),
	m_timeUntilAITurn(1.5f, false),
	m_timeBetweenAIUnits(0.3f, false),
	m_lightIntensityTimer(30.0f),
	m_currentLightIntensity(eLightIntensity::eMaximum)
{
	m_explosionParticles.reserve(6);
	m_fireParticles.reserve(6);
	for (int i = 0; i < 6; i++)
	{
		m_explosionParticles.emplace_back(0.10, Textures::m_explosionParticles, 2.5f);
		m_fireParticles.emplace_back(0.05, Textures::m_fireParticles, 2.0f);
	}
	GameEventMessenger::getInstance().subscribe(std::bind(&Battle::onResetBattle, this), "Battle", GameEvent::eResetBattle);
	GameEventMessenger::getInstance().subscribe(std::bind(&Battle::onYellowShipDestroyed, this), "Battle", GameEvent::eYellowShipDestroyed);
	GameEventMessenger::getInstance().subscribe(std::bind(&Battle::onRedShipDestroyed, this), "Battle", GameEvent::eRedShipDestroyed);
	GameEventMessenger::getInstance().subscribe(std::bind(&Battle::onBlueShipDestroyed, this), "Battle", GameEvent::eBlueShipDestroyed);
	GameEventMessenger::getInstance().subscribe(std::bind(&Battle::onGreenShipDestroyed, this), "Battle", GameEvent::eGreenShipDestroyed);
	GameEventMessenger::getInstance().subscribe(std::bind(&Battle::onEndMovementPhaseEarly, this), "Battle", GameEvent::eEndMovementPhaseEarly);
	GameEventMessenger::getInstance().subscribe(std::bind(&Battle::onEndAttackPhaseEarly, this), "Battle", GameEvent::eEndAttackPhaseEarly);
}

Battle::~Battle()
{
	GameEventMessenger::getInstance().unsubscribe("Battle", GameEvent::eResetBattle);
	GameEventMessenger::getInstance().unsubscribe("Battle", GameEvent::eYellowShipDestroyed);
	GameEventMessenger::getInstance().unsubscribe("Battle", GameEvent::eRedShipDestroyed);
	GameEventMessenger::getInstance().unsubscribe("Battle", GameEvent::eBlueShipDestroyed);
	GameEventMessenger::getInstance().unsubscribe("Battle", GameEvent::eGreenShipDestroyed);
	GameEventMessenger::getInstance().unsubscribe("Battle", GameEvent::eEndMovementPhaseEarly);
	GameEventMessenger::getInstance().unsubscribe("Battle", GameEvent::eEndAttackPhaseEarly);
}

void Battle::start(const std::string & newMapName)
{
	assert(!m_factions.empty());

	m_map.loadmap(newMapName);
	m_battleUI.loadGUI(m_map.getDimensions());
	
	//Assign Spawn Position
	for (auto& player : m_factions)
	{
		if (player)
		{
			player->createSpawnArea(m_map);
		}
	}

	//Set initial deployment state
	bool humanPlayerFound = false;
	for (const auto& player : m_factions)
	{
		if (player && player->m_playerType == ePlayerType::eHuman)
		{
			humanPlayerFound = true;
		}
	}

	if (humanPlayerFound)
	{
		m_currentDeploymentState = eDeploymentState::DeployHuman;
	}
	else
	{
		m_currentDeploymentState = eDeploymentState::DeployAI;
	}
}

void Battle::render() const
{
	m_map.drawMap(m_currentLightIntensity);

	m_battleUI.renderUI();

	for (const auto& player : m_factions)
	{
		if (player)
		{
			player->render(m_map);
		}
	}

	m_battleUI.drawTargetArea();
	
	for (const auto& explosionParticle : m_explosionParticles)
	{
		explosionParticle.render();
	}
	for (const auto& fireParticle : m_fireParticles)
	{
		fireParticle.render();
	}

	m_battleUI.renderGUI();
}

void Battle::update(float deltaTime)
{
	if (m_winningFactionHandler.isGameOver())
	{
		return;
	}

	m_battleUI.setCurrentFaction(getCurrentFaction());
	m_battleUI.update(deltaTime);
	m_map.setDrawOffset(m_battleUI.getCameraPositionOffset());

	if (!m_battleUI.isPaused())
	{
		updateLightIntensity(deltaTime);
		for (auto& explosionParticle : m_explosionParticles)
		{
			explosionParticle.update(deltaTime, m_map);
		}
		for (auto& fireParticle : m_fireParticles)
		{
			fireParticle.update(deltaTime, m_map);
		}

		if (m_currentBattlePhase == BattlePhase::Movement)
		{
			updateMovementPhase(deltaTime);
			if (m_factions[m_currentFactionTurn]->m_playerType == ePlayerType::eAI)
			{
				handleAIMovementPhaseTimer(deltaTime);
			}			
		}
		else if (m_currentBattlePhase == BattlePhase::Attack)
		{
			updateAttackPhase();
			if (m_factions[m_currentFactionTurn]->m_playerType == ePlayerType::eAI)
			{
				handleAIAttackPhaseTimer(deltaTime);
			}
		}
	}

	m_winningFactionHandler.update(deltaTime);
}

void Battle::moveFactionShipToPosition(ShipOnTile shipOnTile, std::pair<int, int> destination)
{
	assert(m_currentBattlePhase == BattlePhase::Movement);
	getFaction(shipOnTile.factionName)->moveShipToPosition(m_map, shipOnTile.shipID, destination);
}

void Battle::moveFactionShipToPosition(ShipOnTile shipOnTile, std::pair<int, int> destination, eDirection endDirection)
{
	assert(m_currentBattlePhase == BattlePhase::Movement);
	getFaction(shipOnTile.factionName)->moveShipToPosition(m_map, shipOnTile.shipID, destination, endDirection);
}

void Battle::disableFactionShipMovementPath(ShipOnTile shipOnTile)
{
	assert(m_currentBattlePhase == BattlePhase::Movement);
	getFaction(shipOnTile.factionName)->disableShipMovementPath(shipOnTile.shipID);
}

void Battle::generateFactionShipMovementPath(ShipOnTile shipOnTile, std::pair<int, int> destination)
{
	assert(m_currentBattlePhase == BattlePhase::Movement);
	getFaction(shipOnTile.factionName)->generateShipMovementPath(m_map, shipOnTile.shipID, destination);
}

void Battle::deployFactionShipAtPosition(std::pair<int, int> startingPosition, eDirection startingDirection)
{
	assert(m_currentBattlePhase == BattlePhase::Deployment);
	m_factions[m_currentFactionTurn]->deployShipAtPosition(m_map, startingPosition, startingDirection);

	if (m_factions[m_currentFactionTurn]->isAllShipsDeployed())
	{
		nextTurn();
	}
}

bool Battle::setShipDeploymentAtPosition(std::pair<int, int> position)
{
	assert(m_currentBattlePhase == BattlePhase::Deployment);
	m_factions[m_currentFactionTurn]->setShipDeploymentAtPosition(position);

	//auto iter = std::find_if(currentPlayer->m_spawnArea.cbegin(), currentPlayer->m_spawnArea.cend(), 
	//	[position](const auto& spawnArea) { return position == spawnArea.m_position; });
	//if (iter != currentPlayer->m_spawnArea.cend())
	//{
	//	currentPlayer->m_shipToDeploy->setDeploymentPosition(position, *this);
	//	return true;
	//}
	//else
	//{
	//	currentPlayer->m_shipToDeploy->setDeploymentPosition(position, *this);
	//	return false;
	//}
}

bool Battle::fireEntityWeaponAtPosition(const Tile& tileOnPlayer, const Tile& tileOnAttackPosition, const std::vector<const Tile*>& targetArea)
{
	assert(m_currentBattlePhase == BattlePhase::Attack);
	assert(tileOnPlayer.m_shipOnTile.isValid());
	assert(!getFactionShip(tileOnPlayer.m_shipOnTile).isWeaponFired());
	
	if (!tileOnPlayer.m_shipOnTile.isValid())
	{
		return false;
	}

	//Disallow attacking same team
	if (tileOnAttackPosition.m_shipOnTile.factionName != getCurrentFaction()
			&& !getFactionShip(tileOnAttackPosition.m_shipOnTile).isDead())
	{
		//Find entity 
		auto tileCoordinate =  getFactionShip(tileOnAttackPosition.m_shipOnTile).getCurrentPosition();
		auto cIter = std::find_if(targetArea.cbegin(), targetArea.cend(), [tileCoordinate](const auto& tile) { if(tile) return tileCoordinate == tile->m_tileCoordinate; });
		//Enemy within range of weapon
		if (cIter != targetArea.cend())
		{
			if (getFactionShip(tileOnPlayer.m_shipOnTile).getShipType() == eShipType::eFire)
			{
				playFireAnimation(getFactionShip(tileOnPlayer.m_shipOnTile), targetArea[0]->m_tileCoordinate);
			}
			else
			{
				playExplosionAnimation(getFactionShip(tileOnAttackPosition.m_shipOnTile));
			}

			ShipOnTile enemy = tileOnAttackPosition.m_shipOnTile;
			m_factions[static_cast<int>(enemy.factionName)]->shipTakeDamage(
				tileOnPlayer.m_shipOnTile.shipID, getFactionShip(enemy).getDamage());
		
			
			return true;
		}
	}

	return false;
}

void Battle::nextTurn()
{
	if (m_currentBattlePhase == BattlePhase::Deployment)
	{
		bool allPlayersDeployed = true;
		bool allHumansDeployed = true;
		if (m_currentDeploymentState == eDeploymentState::DeployHuman)
		{
			for (int i = 0; i < m_factions.size(); ++i)
			{
				if (!m_factions[i])
				{
					continue;
				}

				if (m_factions[i]->m_playerType != ePlayerType::eHuman)
				{
					continue;
				}

				//If first ship hasn't been deployed - rest haven't
				//Proceed to deploy this human player ships
				if (!m_factions[i]->m_ships[0].isDeployed())
				{
					m_currentFactionTurn = i;
					allPlayersDeployed = false;
					allHumansDeployed = false;
					break;
				}
			}

			if (allHumansDeployed)
			{
				m_currentDeploymentState = eDeploymentState::DeployAI;
			}
		}
		
		if (m_currentDeploymentState == eDeploymentState::DeployAI)
		{
			for (int i = 0; i < m_factions.size(); ++i)
			{
				for (!m_factions[i])
				{
					continue;
				}

				if (m_factions[i]->m_playerType != ePlayerType::eAI)
				{
					continue;
				}

				//If first ship hasn't been deployed - rest haven't
				//Proceed to deploy this human player ships
				if (!m_factions[i]->m_ships[0].isDeployed())
				{
					m_currentFactionTurn = i;
					AI::handleDeploymentPhase(*this, m_map, *m_factions[m_currentFactionTurn].get());
					allPlayersDeployed = false;
					break;
				}
			}
		}

		if (allPlayersDeployed)
		{
			switchToBattlePhase(BattlePhase::Movement);
			m_currentFactionTurn = 0;
			GameEventMessenger::getInstance().broadcast(GameEvent::eNewTurn);
			for (auto& ship : m_factions[m_currentFactionTurn]->m_ships)
			{
				ship.enableAction();
			}

			for (auto& player : m_factions)
			{
				player->m_spawnArea.clear();
			}

			if (m_factions[m_currentFactionTurn]->m_playerType == ePlayerType::eAI)
			{
				m_timeUntilAITurn.setActive(true);
				GameEventMessenger::getInstance().broadcast(GameEvent::eEnteredAITurn);
			}
		}
	}
	else if (m_currentBattlePhase == BattlePhase::Movement)
	{
		switchToBattlePhase(BattlePhase::Attack);
		GameEventMessenger::getInstance().broadcast(GameEvent::eNewTurn);

		for (auto& entity : m_factions[m_currentFactionTurn]->m_ships)
		{
			entity.enableAction();
		}

		if (!m_factions[m_currentFactionTurn]->m_eliminated && m_factions[m_currentFactionTurn]->m_playerType == ePlayerType::eAI)
		{
			m_timeUntilAITurn.setActive(true);
			GameEventMessenger::getInstance().broadcast(GameEvent::eEnteredAITurn);
		}
	}
	else if (m_currentBattlePhase == BattlePhase::Attack)
	{
		switchToBattlePhase(BattlePhase::Movement);
		GameEventMessenger::getInstance().broadcast(GameEvent::eNewTurn);

		for (auto& entity : m_factions[m_currentFactionTurn]->m_ships)
		{
			entity.disableAction();
		}

		updateWindDirection();
		incrementPlayerTurn();
		for (auto& entity : m_factions[m_currentFactionTurn]->m_ships)
		{
			entity.enableAction();
		}

		if (!m_factions[m_currentFactionTurn]->m_eliminated && m_factions[m_currentFactionTurn]->m_playerType == ePlayerType::eAI)
		{
			m_timeUntilAITurn.setActive(true);
			GameEventMessenger::getInstance().broadcast(GameEvent::eEnteredAITurn);
		}
		else if (m_factions[m_currentFactionTurn]->m_playerType == ePlayerType::eHuman)
		{
			GameEventMessenger::getInstance().broadcast(GameEvent::eLeftAITurn);
		}
	}
}

void Battle::switchToBattlePhase(BattlePhase newBattlePhase)
{
	m_currentBattlePhase = newBattlePhase;
	m_battleUI.onEnteringBattlePhase(m_currentBattlePhase);
}

void Battle::notifyPlayersOnNewTurn()
{
	for (auto& player : m_factions)
	{
		player->onNewTurn();
	}
}

std::vector<FactionName> Battle::getAllFactionsInPlay() const
{
	std::vector<FactionName> allFactionsInPlay;
	allFactionsInPlay.reserve(m_factions.size());
	for (const auto& player : m_factions)
	{
		allFactionsInPlay.push_back(player->m_factionName);
	}

	assert(!allFactionsInPlay.empty());
	return allFactionsInPlay;
}

void Battle::playFireAnimation(const Ship& entity, std::pair<int, int> position)
{
	for (auto& it : m_fireParticles)
	{
		if (!it.m_isEmitting)
		{
			it.orient(entity.getCurrentDirection());
			it.setPosition(position);
			it.m_isEmitting = true;
			return;
		}
	}
}

void Battle::playExplosionAnimation(const Ship& entity)
{
	for (auto& it : m_explosionParticles)
	{
		if (!it.m_isEmitting)
		{
			it.setPosition(entity.getCurrentPosition());
			it.m_isEmitting = true;
			return;
		}
	}
}

void Battle::updateLightIntensity(float deltaTime)
{
	m_lightIntensityTimer.update(deltaTime);
	if (m_lightIntensityTimer.isExpired())
	{
		if (m_currentLightIntensity == eLightIntensity::eMaximum)
		{
			m_currentLightIntensity = eLightIntensity::eMinimum;
			m_lightIntensityTimer.setNewExpirationTime(15.f);
			m_lightIntensityTimer.reset();
		}
		else
		{
			m_currentLightIntensity = eLightIntensity::eMaximum;
			m_lightIntensityTimer.setNewExpirationTime(30.f);
			m_lightIntensityTimer.reset();
		}
	}
}

void Battle::updateMovementPhase(float deltaTime)
{
	for (auto& entity : m_factions[m_currentFactionTurn]->m_ships)
	{
		if (entity.isDead())
			continue;
		entity.update(deltaTime, m_map);
	}
	for (auto& entity : m_factions[m_currentFactionTurn]->m_ships)
	{
		if (entity.isDead())
			continue;
		if (!entity.isDestinationSet())
		{
			return;
		}
		if (entity.isMovingToDestination())
		{
			return;
		}
	}

	nextTurn();
}

void Battle::updateAttackPhase()
{
	bool allEntitiesAttacked = true;
	for (const auto& ship : m_factions[m_currentFactionTurn]->m_ships)
	{
		if (!ship.isDead() && !ship.isWeaponFired())
		{
			allEntitiesAttacked = false;
		}
	}

	if (allEntitiesAttacked)
	{	
		nextTurn();
	}
}

Faction& Battle::getPlayer(FactionName factionName)
{
	assert(m_factions[static_cast<int>(factionName)]);
	return m_factions[static_cast<int>(factionName)].get();
	auto cIter = std::find_if(m_factions.begin(), m_factions.end(), [factionName](auto& player) { return factionName == player->m_factionName; });
	assert(cIter != m_factions.end());
	return *cIter->get();
}

std::unique_ptr<Faction>& Battle::getFaction(FactionName factionName)
{
	assert(m_factions[static_cast<int>(factionName)].get());
	return m_factions[static_cast<int>(factionName)];
}

std::unique_ptr<Faction>& Battle::getCurrentPlayer()
{
	assert(m_factions[m_currentFactionTurn].get());
	return m_factions[m_currentFactionTurn];
}

void Battle::onResetBattle()
{
	m_currentBattlePhase = BattlePhase::Deployment;
	m_currentFactionTurn = 0;
	m_currentLightIntensity = eLightIntensity::eMaximum;
	m_lightIntensityTimer.reset();
}

void Battle::incrementPlayerTurn()
{
	int wind = rand() % eDirection::Max;
	m_map.setWindDirection((eDirection)wind);
	
	++m_currentFactionTurn;
	if (m_currentFactionTurn == static_cast<int>(m_factions.size()))
	{
		m_currentFactionTurn = 0;
	}
}

const Map & Battle::getMap() const
{
	return m_map;
}

BattlePhase Battle::getCurrentPhase() const
{
	return m_currentBattlePhase;
}

FactionName Battle::getCurrentFaction() const
{
	return m_factions[m_currentFactionTurn]->m_factionName;
}

ePlayerType Battle::getCurrentPlayerType() const
{
	assert(m_factions[m_currentFactionTurn].get());
	return m_factions[m_currentFactionTurn]->m_playerType;
}

const Faction & Battle::getPlayer(FactionName factionName) const
{
	auto cIter = std::find_if(m_factions.cbegin(), m_factions.cend(), [factionName](auto& player) { return player->m_factionName == factionName; });
	assert(cIter != m_factions.cend());
	return *cIter->get();
}

const Ship & Battle::getFactionShip(ShipOnTile shipOnTile) const
{
	assert(shipOnTile.factionName != FactionName::Invalid);
	assert(shipOnTile.shipID != INVALID_SHIP_ID);

	FactionName factionName = shipOnTile.factionName;
	auto player = std::find_if(m_factions.cbegin(), m_factions.cend(), [factionName](auto& player) { return player->m_factionName == factionName; });
	assert(player != m_factions.cend());
	player->get()->m_ships[shipOnTile.shipID];
}

void Battle::onYellowShipDestroyed()
{
	m_winningFactionHandler.onYellowShipDestroyed(m_factions);
}

void Battle::onBlueShipDestroyed()
{
	m_winningFactionHandler.onBlueShipDestroyed(m_factions);
}

void Battle::onGreenShipDestroyed()
{
	m_winningFactionHandler.onGreenShipDestroyed(m_factions);
}

void Battle::onRedShipDestroyed()
{
	m_winningFactionHandler.onRedShipDestroyed(m_factions);
}

void Battle::onEndMovementPhaseEarly()
{
	bool actionBeingPerformed = false;
	for (auto& entity : m_factions[m_currentFactionTurn]->m_ships)
	{
		if (entity.isMovingToDestination())
		{
			actionBeingPerformed = true;
		}
	}
	if (actionBeingPerformed)
	{
		GameEventMessenger::getInstance().broadcast(GameEvent::eUnableToSkipPhase);
	}
	else
	{
		m_battleUI.clearSelectedTile();
		nextTurn();
	}
}

void Battle::onEndAttackPhaseEarly()
{
	nextTurn();
}

Battle::WinningFactionHandler::WinningFactionHandler()
	: m_yellowShipsDestroyed(0),
	m_blueShipsDestroyed(0),
	m_greenShipsDestroyed(0),
	m_redShipsDestroyed(0),
	m_winTimer(2.0f, false),
	m_gameOver(false)
{
	GameEventMessenger::getInstance().subscribe(std::bind(&WinningFactionHandler::onReset, this), "WinningFactionHandler", GameEvent::eResetBattle);
}

Battle::WinningFactionHandler::~WinningFactionHandler()
{
	GameEventMessenger::getInstance().unsubscribe("WinningFactionHandler", GameEvent::eResetBattle);
}

bool Battle::WinningFactionHandler::isGameOver() const
{
	return m_gameOver;
}

void Battle::WinningFactionHandler::update(BattleUI& battleUI, float deltaTime)
{
	m_winTimer.update(deltaTime);
	if (m_winTimer.isExpired())
	{
		switch (m_winningFaction)
		{
		case FactionName::eYellow :
			battleUI.onFactionWin(FactionName::eYellow);
			break;
		case FactionName::eBlue :
			battleUI.onFactionWin(FactionName::eBlue);
			break;
		case FactionName::eRed :
			battleUI.onFactionWin(FactionName::eRed);
			break;
		case FactionName::eGreen :
			battleUI.onFactionWin(FactionName::eGreen);
			break;
		}

		m_gameOver = true;
	}
}

void Battle::WinningFactionHandler::onYellowShipDestroyed(std::array<std::unique_ptr<Faction>, static_cast<size_t>(FactionName::MAX)>& players)
{
	++m_yellowShipsDestroyed;
	auto player = std::find_if(players.begin(), players.end(), [](auto& player) { return player->m_factionName == FactionName::eYellow; });
	assert(player != players.end());
	if (m_yellowShipsDestroyed == static_cast<int>(player->get()->m_ships.size()))
	{
		player->get()->m_eliminated = true;
		checkGameStatus(players);
	}
}

void Battle::WinningFactionHandler::onBlueShipDestroyed(std::array<std::unique_ptr<Faction>, static_cast<size_t>(FactionName::MAX)>& players)
{
	++m_blueShipsDestroyed;
	auto player = std::find_if(players.begin(), players.end(), [](auto& player) { return player->m_factionName == FactionName::eBlue; });
	assert(player != players.end());
	if (m_blueShipsDestroyed == static_cast<int>(player->get()->m_ships.size()))
	{
		player->get()->m_eliminated = true;
		checkGameStatus(players);
	}
}

void Battle::WinningFactionHandler::onGreenShipDestroyed(std::array<std::unique_ptr<Faction>, static_cast<size_t>(FactionName::MAX)>& players)
{
	++m_greenShipsDestroyed;
	auto player = std::find_if(players.begin(), players.end(), [](auto& player) { return player->m_factionName == FactionName::eGreen; });
	assert(player != players.end());
	if (m_greenShipsDestroyed == static_cast<int>(player->get()->m_ships.size()))
	{
		player->get()->m_eliminated = true;
		checkGameStatus(players);
	}
}

void Battle::WinningFactionHandler::onRedShipDestroyed(std::array<std::unique_ptr<Faction>, static_cast<size_t>(FactionName::MAX)>& players)
{
	++m_redShipsDestroyed;
	auto player = std::find_if(players.begin(), players.end(), [](auto& player) { return player->m_factionName == FactionName::eRed; });
	assert(player != players.end());
	if (m_redShipsDestroyed == static_cast<int>(player->get()->m_ships.size()))
	{
		player->get()->m_eliminated = true;
		checkGameStatus(players);
	}
}

void Battle::WinningFactionHandler::onReset()
{
	m_yellowShipsDestroyed = 0;
	m_redShipsDestroyed = 0;
	m_blueShipsDestroyed = 0;
	m_greenShipsDestroyed = 0;
	m_winTimer.reset();
	m_winTimer.setActive(false);
	m_gameOver = false;
}

void Battle::WinningFactionHandler::checkGameStatus(const std::array<std::unique_ptr<Faction>, static_cast<size_t>(FactionName::MAX)>& factions)
{
	//Check to see if all players have been eliminated
	int playersEliminated = 0;
	for (const auto& faction : factions)
	{
		if (faction && faction->m_eliminated)
		{
			++playersEliminated;
		}
	}
	//Last player standing - Player wins
	if (playersEliminated == static_cast<int>(factions.size()) - 1)
	{
		FactionName winningFaction;
		for (const auto& faction : factions)
		{
			if (faction && !faction->m_eliminated)
			{
				winningFaction = faction->m_factionName;
			}
		}
		switch (winningFaction)
		{
		case FactionName::eYellow:
			m_winningFaction = FactionName::eYellow;
			break;
		case FactionName::eBlue:
			m_winningFaction = FactionName::eBlue;
			break;
		case FactionName::eGreen:
			m_winningFaction = FactionName::eGreen;
			break;
		case FactionName::eRed:
			m_winningFaction = FactionName::eRed;
			break;
		}
		m_winTimer.setActive(true);
	}
}