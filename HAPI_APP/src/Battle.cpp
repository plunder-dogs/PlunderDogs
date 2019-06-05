#include "Battle.h"
#include "Utilities/MapParser.h"
#include "GameEventMessenger.h"
#include "AI.h"
#include "Textures.h"

Battle::Particle::Particle(float lifespan, std::unique_ptr<sf::Texture>& texture, float scale) 
	: m_position(),
	m_lifeSpan(lifespan),
	m_sprite(*texture.get()),
	m_frameNum(0),
	m_isEmitting(false),
	m_scale(scale)
{
	//m_particle->SetFrameNumber(m_frameNum);
}

void Battle::Particle::setPosition(sf::Vector2i position)
{
	m_position = position;
}

void Battle::Particle::update(float deltaTime, const Map& map)
{
	if (m_isEmitting)
	{
		const sf::Vector2i tileTransform = map.getTileScreenPos(m_position);
		m_sprite.setPosition({
			tileTransform.x + DRAW_OFFSET_X * map.getDrawScale(),
			tileTransform.y + DRAW_OFFSET_Y * map.getDrawScale() });

		m_lifeSpan.update(deltaTime);

		if (m_lifeSpan.isExpired())
		{
			//m_sprite->SetFrameNumber(m_frameNum);

			m_lifeSpan.reset();
			++m_frameNum;
		}

		//if (m_frameNum >= m_sprite->GetNumFrames())
		//{
		//	m_isEmitting = false;
		//	m_frameNum = 0;
		//}
	}
}

void Battle::Particle::render(sf::RenderWindow& window) 
{
	if (m_isEmitting)
	{
		//m_sprite->GetTransformComp().SetOriginToCentreOfFrame();
		m_sprite.setScale(m_scale, m_scale);
		window.draw(m_sprite);
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

#ifdef SFML_REFACTOR - Ryan Swann
	m_sprite->GetTransformComp().SetRotation(DEGREES_TO_RADIANS(static_cast<int>(direction) * 60 % 360));
#endif 
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

Battle::Battle(std::array<std::unique_ptr<Faction>, static_cast<size_t>(FactionName::eTotal)>& players)
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
	for (auto& faction : m_factions)
	{
		if (faction)
		{
			faction->createSpawnArea(m_map);
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

void Battle::render(sf::RenderWindow& window)
{
	m_map.drawMap(window, m_currentLightIntensity);

	m_battleUI.renderUI(window);

	for (auto& faction : m_factions)
	{
		if (faction)
		{
			faction->render(window, m_map);
		}
	}

	m_battleUI.drawTargetArea(window);
	
	
	for (auto& explosionParticle : m_explosionParticles)
	{
		explosionParticle.render(window);
	}
	for (auto& fireParticle : m_fireParticles)
	{
		fireParticle.render(window);
	}

	m_battleUI.renderGUI(window);
}

void Battle::handleInput(const sf::Event & currentEvent)
{
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

	m_winningFactionHandler.update(m_battleUI, deltaTime);
}

void Battle::moveFactionShipToPosition(ShipOnTile shipOnTile, sf::Vector2i destination)
{
	assert(m_currentBattlePhase == BattlePhase::Movement);
	getFaction(shipOnTile.factionName).moveShipToPosition(m_map, shipOnTile.shipID, destination);
}

void Battle::moveFactionShipToPosition(ShipOnTile shipOnTile, sf::Vector2i destination, eDirection endDirection)
{
	assert(m_currentBattlePhase == BattlePhase::Movement);
	getFaction(shipOnTile.factionName).moveShipToPosition(m_map, shipOnTile.shipID, destination, endDirection);
}

void Battle::disableFactionShipMovementPath(ShipOnTile shipOnTile)
{
	assert(m_currentBattlePhase == BattlePhase::Movement);
	getFaction(shipOnTile.factionName).disableShipMovementPath(shipOnTile.shipID);
}

void Battle::generateFactionShipMovementPath(ShipOnTile shipOnTile, sf::Vector2i destination)
{
	assert(m_currentBattlePhase == BattlePhase::Movement);
	getFaction(shipOnTile.factionName).generateShipMovementPath(m_map, shipOnTile.shipID, destination);
}

void Battle::deployFactionShipAtPosition(sf::Vector2i startingPosition, eDirection startingDirection)
{
	assert(m_currentBattlePhase == BattlePhase::Deployment);
	m_factions[m_currentFactionTurn]->deployShipAtPosition(m_map, startingPosition, startingDirection);

	if (m_factions[m_currentFactionTurn]->isAllShipsDeployed())
	{
		nextTurn();
	}
}

bool Battle::setShipDeploymentAtPosition(sf::Vector2i position)
{
	assert(m_currentBattlePhase == BattlePhase::Deployment);
	return m_factions[m_currentFactionTurn]->setShipDeploymentAtPosition(position);
}

bool Battle::fireFactionShipAtPosition(ShipOnTile firingShip, ShipOnTile enemyShip, const std::vector<const Tile*>& targetArea)
{
	assert(m_currentBattlePhase == BattlePhase::Attack);
	assert(firingShip.isValid());
	assert(!getFactionShip(firingShip).isWeaponFired());

	const Ship& firingShipInPlay = getFactionShip(firingShip);
	const Ship& enemyShipInPlay = getFactionShip(enemyShip);

	//Disallow attacking same team
	if (enemyShipInPlay.getFactionName() != getCurrentFaction() && !enemyShipInPlay.isDead())
	{
		//Find Enemy Ship 
		sf::Vector2i enemyShipInPlayPosition = enemyShipInPlay.getCurrentPosition();
		auto cIter = std::find_if(targetArea.cbegin(), targetArea.cend(), 
			[enemyShipInPlayPosition](const auto& tile) { if (tile) return enemyShipInPlayPosition == tile->m_tileCoordinate; });
		//Enemy Ship within range of weapon
		if (cIter != targetArea.cend())
		{
			if (firingShipInPlay.getShipType() == eShipType::eFire)
			{
				playFireAnimation(firingShipInPlay.getCurrentDirection(), targetArea[0]->m_tileCoordinate);
			}
			else
			{
				playExplosionAnimation(enemyShipInPlay.getCurrentPosition());
			}

			getFaction(enemyShip.factionName).shipTakeDamage(enemyShip.shipID, firingShipInPlay.getDamage());
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
				if(!m_factions[i])
				{
					continue;
				}
				for (int i = 0; i < m_factions.size(); ++i)
				{
					if (!m_factions[i])
					{
						continue;
					}
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

			for (auto& faction : m_factions)
			{
				if (faction)
				{
					faction->m_spawnArea.clear();
				}
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
		m_factions[m_currentFactionTurn]->onNewTurn();

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

		m_factions[m_currentFactionTurn]->onNewTurn();
		for (auto& entity : m_factions[m_currentFactionTurn]->m_ships)
		{
			entity.disableAction();
		}

		updateWindDirection();
		incrementFactionTurn();
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
	for (const auto& faction : m_factions)
	{
		if (faction)
		{
			allFactionsInPlay.push_back(faction->m_factionName);
		}
	}

	assert(!allFactionsInPlay.empty());
	return allFactionsInPlay;
}

void Battle::playFireAnimation(eDirection orientation, sf::Vector2i position)
{
	for (auto& it : m_fireParticles)
	{
		if (!it.m_isEmitting)
		{
			it.orient(orientation);
			it.setPosition(position);
			it.m_isEmitting = true;
			break;
		}
	}
}

void Battle::playExplosionAnimation(sf::Vector2i position)
{
	for (auto& it : m_explosionParticles)
	{
		if (!it.m_isEmitting)
		{
			it.setPosition(position);
			it.m_isEmitting = true;
			break;
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
	for (auto& ship : m_factions[m_currentFactionTurn]->m_ships)
	{
		if (ship.isDead())
			continue;
		ship.update(deltaTime, m_map);
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

std::unique_ptr<Faction>& Battle::getCurrentPlayer()
{
	assert(m_factions[m_currentFactionTurn].get());
	return m_factions[m_currentFactionTurn];
}

Faction & Battle::getFaction(FactionName factionName)
{
	assert(m_factions[m_currentFactionTurn].get());
	return *m_factions[m_currentFactionTurn];
}

void Battle::onResetBattle()
{
	m_currentBattlePhase = BattlePhase::Deployment;
	m_currentFactionTurn = 0;
	m_currentLightIntensity = eLightIntensity::eMaximum;
	m_lightIntensityTimer.reset();
}

void Battle::incrementFactionTurn()
{
	//Change wind direction
	int wind = rand() % eDirection::Max;
	m_map.setWindDirection((eDirection)wind);
	
	//Select first faction
	if (m_currentFactionTurn == static_cast<int>(m_factions.size()) - 1)
	{
		for (int i = 0; i < m_factions.size(); ++i)
		{
			if (m_factions[i])
			{
				m_currentFactionTurn = i;
				return;
			}
		}
	}

	//Select next faction
	for (int i = m_currentFactionTurn + 1; i < m_factions.size(); ++i)
	{
		if (m_factions[i])
		{
			m_currentFactionTurn = i;
			return;
		}
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

const Ship & Battle::getFactionShip(ShipOnTile shipOnTile) const
{
	assert(shipOnTile.isValid());
	return m_factions[static_cast<int>(shipOnTile.factionName)]->getShip(shipOnTile.shipID);
}

const Faction & Battle::getFaction(FactionName factionName) const
{
	assert(m_factions[static_cast<int>(factionName)]);
	return *m_factions[static_cast<int>(factionName)].get();
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
	for (const auto& entity : m_factions[m_currentFactionTurn]->m_ships)
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

void Battle::WinningFactionHandler::onYellowShipDestroyed(std::array<std::unique_ptr<Faction>, static_cast<size_t>(FactionName::eTotal)>& players)
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

void Battle::WinningFactionHandler::onBlueShipDestroyed(std::array<std::unique_ptr<Faction>, static_cast<size_t>(FactionName::eTotal)>& factions)
{
	++m_blueShipsDestroyed;
	assert(factions[static_cast<int>(FactionName::eBlue)]);
	if (m_greenShipsDestroyed == (int)factions[static_cast<int>(FactionName::eBlue)]->m_ships.size())
	{
		factions[static_cast<int>(FactionName::eBlue)]->m_eliminated = true;
		checkGameStatus(factions);
	}
}

void Battle::WinningFactionHandler::onGreenShipDestroyed(std::array<std::unique_ptr<Faction>, static_cast<size_t>(FactionName::eTotal)>& players)
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

void Battle::WinningFactionHandler::onRedShipDestroyed(std::array<std::unique_ptr<Faction>, static_cast<size_t>(FactionName::eTotal)>& players)
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

void Battle::WinningFactionHandler::checkGameStatus(const std::array<std::unique_ptr<Faction>, static_cast<size_t>(FactionName::eTotal)>& factions)
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