#include "Battle.h"
#include "Utilities/XMLParser.h"
#include "GameEventMessenger.h"
#include "AI.h"
#include "Textures.h"
#include <iostream>

constexpr size_t MAX_PARTICLES = 6;

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
	m_explosionParticles.reserve(MAX_PARTICLES);
	m_fireParticles.reserve(MAX_PARTICLES);
	for (int i = 0; i < MAX_PARTICLES; i++)
	{
		m_explosionParticles.emplace_back(0.10, Textures::getInstance().m_explosionParticles, 2.5f);
		m_fireParticles.emplace_back(0.05, Textures::getInstance().m_fireParticles, 2.0f);
	}

	GameEventMessenger::getInstance().subscribe(std::bind(&Battle::onEndBattlePhaseEarly, this), GameEvent::eEndBattlePhaseEarly);
}

Battle::~Battle()
{
	GameEventMessenger::getInstance().unsubscribe(GameEvent::eEndBattlePhaseEarly);
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
			break;
		}
	}

	if (humanPlayerFound)
	{
		m_currentDeploymentState = eDeploymentState::DeployingPlayer;
	}
	else
	{
		m_currentDeploymentState = eDeploymentState::DeployingAI;
	}
}

void Battle::render(sf::RenderWindow& window)
{
	m_map.renderMap(window);
	
	for (auto& faction : m_factions)
	{
		if (faction)
		{
			faction->render(window, m_map, m_currentBattlePhase);
		}
	}
	
	m_battleUI.render(window);
	
	for (auto& explosionParticle : m_explosionParticles)
	{
		explosionParticle.render(window, m_map);
	}
	for (auto& fireParticle : m_fireParticles)
	{
		fireParticle.render(window, m_map);
	}
}

void Battle::renderFactionShipsMovementGraphs(sf::RenderWindow & window)
{
	if (m_currentBattlePhase == BattlePhase::Movement)
	{
		assert(m_factions[m_currentFactionTurn].get());
		return m_factions[m_currentFactionTurn]->renderShipsMovementGraphs(window, m_map);
	}
}

void Battle::handleInput(const sf::RenderWindow& window, const sf::Event & currentEvent)
{
	m_battleUI.handleInput(window, currentEvent);
}

void Battle::update(float deltaTime)
{
	m_battleUI.update(deltaTime);
	m_map.setDrawOffset(m_battleUI.getCameraPositionOffset());
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

void Battle::disableFactionShipMovementGraph(ShipOnTile shipOnTile)
{
	assert(m_currentBattlePhase == BattlePhase::Movement);
	getFaction(shipOnTile.factionName).disableShipMovementGraph(shipOnTile.shipID);
}

void Battle::generateFactionShipMovementGraph(ShipOnTile shipOnTile, sf::Vector2i destination)
{
	assert(m_currentBattlePhase == BattlePhase::Movement);
	getFaction(shipOnTile.factionName).generateShipMovementGraph(m_map, shipOnTile.shipID, destination);
}

void Battle::deployFactionShipAtPosition(sf::Vector2i startingPosition, eDirection startingDirection)
{
	assert(m_currentBattlePhase == BattlePhase::Deployment);
	m_factions[m_currentFactionTurn]->deployShipAtPosition(m_map, startingPosition, startingDirection);

	if (m_factions[m_currentFactionTurn]->isAllShipsDeployed())
	{
		advanceToNextBattlePhase();
	}
}

void Battle::setShipDeploymentAtPosition(sf::Vector2i position)
{
	assert(m_currentBattlePhase == BattlePhase::Deployment);
	m_factions[m_currentFactionTurn]->setShipDeploymentAtPosition(position);
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
			[enemyShipInPlayPosition](const auto& tile) { return enemyShipInPlayPosition == tile->m_tileCoordinate; });
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

			m_factions[firingShip.factionName]->m_ships[firingShip.shipID].fireWeapon();
			getFaction(enemyShip.factionName).shipTakeDamage(enemyShip.shipID, firingShipInPlay.getDamage());
			return true;
		}
	}

	return false;
}

void Battle::advanceToNextBattlePhase()
{
	if (m_currentBattlePhase == BattlePhase::Deployment)
	{
		bool allPlayersDeployed = true;
		bool allHumansDeployed = true;
		if (m_currentDeploymentState == eDeploymentState::DeployingPlayer)
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
				m_currentDeploymentState = eDeploymentState::DeployingAI;
			}
		}
		
		if (m_currentDeploymentState == eDeploymentState::DeployingAI)
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
					AI::handleDeploymentPhase(*this, *m_factions[m_currentFactionTurn].get());
					allPlayersDeployed = false;
					break;
				}
			}
		}

		if (allPlayersDeployed)
		{
			switchToBattlePhase(BattlePhase::Movement);
			m_currentDeploymentState = eDeploymentState::Finished;
			m_currentFactionTurn = 0;

			for (auto& faction : m_factions)
			{
				if (faction)
				{
					faction->clearSpawnArea();
				}
			}

			for (auto& ship : m_factions[m_currentFactionTurn]->m_ships)
			{
				ship.enableAction();
			}

			if (m_factions[m_currentFactionTurn]->m_playerType == ePlayerType::eAI)
			{
				m_timeUntilAITurn.setActive(true);
				//TODO: Refactor
				//GameEventMessenger::getInstance().broadcast(GameEvent::eEnteredAITurn);
			}
		}
	}
	else if (m_currentBattlePhase == BattlePhase::Movement)
	{
		switchToBattlePhase(BattlePhase::Attack);

		for (auto& entity : m_factions[m_currentFactionTurn]->m_ships)
		{
			entity.enableAction();
		}

		if (!m_factions[m_currentFactionTurn]->isEliminated() && m_factions[m_currentFactionTurn]->m_playerType == ePlayerType::eAI)
		{
			m_timeUntilAITurn.setActive(true);
			//TODO: Refactor
			//GameEventMessenger::getInstance().broadcast(GameEvent::eEnteredAITurn);
		}
	}
	else if (m_currentBattlePhase == BattlePhase::Attack)
	{
		switchToBattlePhase(BattlePhase::Movement);

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

		if (!m_factions[m_currentFactionTurn]->isEliminated() && m_factions[m_currentFactionTurn]->m_playerType == ePlayerType::eAI)
		{
			m_timeUntilAITurn.setActive(true);
			//TODO: Refactor		
			//GameEventMessenger::getInstance().broadcast(GameEvent::eEnteredAITurn);
		}
		else if (m_factions[m_currentFactionTurn]->m_playerType == ePlayerType::eHuman)
		{
			//TODO: Refactor
			//GameEventMessenger::getInstance().broadcast(GameEvent::eLeftAITurn);
		}
	}
}

void Battle::switchToBattlePhase(BattlePhase newBattlePhase)
{
	m_currentBattlePhase = newBattlePhase;
	GameEventMessenger::getInstance().broadcast(GameEvent::eEnteredNewBattlePhase);
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
	for (auto& particle : m_fireParticles)
	{
		if (!particle.m_sprite.isActive())
		{
			particle.orient(orientation);
			particle.setPosition(position);
			particle.m_sprite.activate();
			break;
		}
	}
}

void Battle::playExplosionAnimation(sf::Vector2i position)
{
	for (auto& particle : m_explosionParticles)
	{
		if (!particle.m_sprite.isActive())
		{
			particle.setPosition(position);
			particle.m_sprite.activate();
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
		ship.update(deltaTime, m_map);
	}
	for (const auto& entity : m_factions[m_currentFactionTurn]->m_ships)
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

	advanceToNextBattlePhase();
}

void Battle::updateAttackPhase()
{
	bool allShipsAttacked = true;
	for (const auto& ship : m_factions[m_currentFactionTurn]->m_ships)
	{
		if (!ship.isDead() && !ship.isWeaponFired())
		{
			allShipsAttacked = false;
		}
	}

	if (allShipsAttacked)
	{	
		advanceToNextBattlePhase();
	}
}

std::unique_ptr<Faction>& Battle::getCurrentPlayer()
{
	assert(m_factions[m_currentFactionTurn].get());
	return m_factions[m_currentFactionTurn];
}

Faction & Battle::getFaction(FactionName factionName)
{
	assert(m_factions[static_cast<int>(factionName)].get());
	return *m_factions[static_cast<int>(factionName)];
}

void Battle::incrementFactionTurn()
{
	//Change wind direction
	int wind = rand() % eDirection::Max;
	m_map.setWindDirection((eDirection)wind);

	//Select first faction available
	if (m_currentFactionTurn == static_cast<int>(m_factions.size()) - 1)
	{
		for (int i = 0; i < m_factions.size(); ++i)
		{
			if (m_factions[i])
			{
				m_currentFactionTurn = i;
				break;
			}
		}
	}
	else
	{
		//Select next faction available
		for (int i = m_currentFactionTurn + 1; i < m_factions.size(); ++i)
		{
			if (m_factions[i])
			{
				m_currentFactionTurn = i;
				break;
			}
		}
	}
}

bool Battle::isShipBelongToCurrentFactionInPlay(ShipOnTile shipOnTile) const
{
	assert(m_factions[m_currentFactionTurn].get());
	return shipOnTile.factionName == getCurrentFaction();
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
	assert(m_factions[m_currentFactionTurn].get());
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

void Battle::onEndBattlePhaseEarly()
{
	if (m_currentBattlePhase == BattlePhase::Movement)
	{
		auto& ships = m_factions[m_currentFactionTurn]->m_ships;
		auto cIter = std::find_if(ships.cbegin(), ships.cend(), [](const auto& ship) { return ship.isMovingToDestination(); });
		if (cIter == ships.cend())
		{
			advanceToNextBattlePhase();
		}
	}
	else if (m_currentBattlePhase == BattlePhase::Attack)
	{
		advanceToNextBattlePhase();
	}
}