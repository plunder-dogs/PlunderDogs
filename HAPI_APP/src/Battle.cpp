#include "Battle.h"
#include "Utilities/XMLParser.h"
#include "GameEventMessenger.h"
#include "AI.h"
#include "Textures.h"
#include "NetworkHandler.h"
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

void Battle::handleServerMessages()
{
	//for (auto message : NetworkHandler::getInstance().getServerMessages())
	//{
	//	if (static_cast<eMessageType>(message.type) == eMessageType::eDeployShip)
	//	{
	//		deployFactionShipAtPosition(message.position, message.direction);
	//	}
	//}

	//NetworkHandler::getInstance().getServerMessages().clear();

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
		for (auto& ship : m_factions[m_currentFactionTurn].m_ships)
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
		for (auto& ship : m_factions[m_currentFactionTurn].m_ships)
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

void Battle::handleTimeUntilGameOver(float deltaTime)
{
	m_timeUntilGameOver.update(deltaTime);
	if (m_timeUntilGameOver.isExpired())
	{
		m_isRunning = false;
	}
}

Battle::Battle(std::array<Faction, static_cast<size_t>(FactionName::eTotal)>& players)
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
	m_currentLightIntensity(eLightIntensity::eMaximum),
	m_isRunning(true),
	m_timeUntilGameOver(2.f, false)
{
	m_explosionParticles.reserve(MAX_PARTICLES);
	m_fireParticles.reserve(MAX_PARTICLES);
	for (int i = 0; i < MAX_PARTICLES; i++)
	{
		m_explosionParticles.emplace_back(0.10, *Textures::getInstance().m_explosionParticles, 2.5f);
		m_fireParticles.emplace_back(0.05, *Textures::getInstance().m_fireParticles, 2.0f);
	}

	GameEventMessenger::getInstance().subscribe(std::bind(&Battle::onEndBattlePhaseEarly, this, std::placeholders::_1), eGameEvent::eEndBattlePhaseEarly);
	GameEventMessenger::getInstance().subscribe(std::bind(&Battle::onFactionShipDestroyed, this, std::placeholders::_1), eGameEvent::eFactionShipDestroyed);
}

Battle::~Battle()
{
	GameEventMessenger::getInstance().unsubscribe(eGameEvent::eEndBattlePhaseEarly);
	GameEventMessenger::getInstance().unsubscribe(eGameEvent::eFactionShipDestroyed);
}

void Battle::startOnlineGame(const std::string & newMapName, const std::vector<ServerMessageSpawnPosition>& factionSpawnPositions)
{
	assert(!m_factions.empty());

	m_onlineGame = true;
	m_map.loadmap(newMapName);
	m_battleUI.setMaxCameraOffset(m_map.getDimensions());
	
	for (auto& faction : m_factions)
	{
		if (faction.isActive())
		{
			auto factionName = faction.m_factionName;
			auto cIter = std::find_if(factionSpawnPositions.cbegin(), factionSpawnPositions.cend(), 
				[factionName](const auto& faction) { return faction.factionName == factionName; });
			assert(cIter != factionSpawnPositions.cend());
			
			faction.createSpawnArea(m_map, cIter->position);
		}
	}

	//Set initial deployment state
	bool playerFound = false;
	for (const auto& faction : m_factions)
	{
		if (faction.isActive() && faction.m_controllerType == eControllerType::eLocalPlayer ||
			faction.m_controllerType == eControllerType::eRemotePlayer)
		{
			playerFound = true;
			break;
		}
	}

	if (playerFound)
	{
		m_currentDeploymentState = eDeploymentState::DeployingPlayer;
	}
	else
	{
		m_currentDeploymentState = eDeploymentState::DeployingAI;
	}
}

void Battle::startSinglePlayerGame(const std::string & levelName)
{
	assert(!m_factions.empty());

	m_onlineGame = false;
	m_map.loadmap(levelName);
	m_battleUI.setMaxCameraOffset(m_map.getDimensions());

	for (auto& faction : m_factions)
	{
		if (faction.isActive())
		{
			faction.createSpawnArea(m_map, m_map.getRandomSpawnPosition());
		}
	}

	//Set initial deployment state
	bool playerFound = false;
	for (const auto& faction : m_factions)
	{
		if (faction.isActive() && faction.m_controllerType == eControllerType::eLocalPlayer)
		{
			playerFound = true;
			break;
		}
	}

	if (playerFound)
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
		if (faction.isActive())
		{
			faction.render(window, m_map, m_currentBattlePhase);
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
		assert(m_factions[m_currentFactionTurn].isActive());
		return m_factions[m_currentFactionTurn].renderShipsMovementGraphs(window, m_map);
	}
}

void Battle::handleInput(const sf::RenderWindow& window, const sf::Event & currentEvent)
{
	if (m_factions[m_currentFactionTurn].m_controllerType == eControllerType::eRemotePlayer)
	{
		return;
	}
	
	m_battleUI.handleInput(window, currentEvent);
}

void Battle::update(float deltaTime)
{
	m_battleUI.update(deltaTime);
	m_map.setDrawOffset(m_battleUI.getCameraPositionOffset());
	updateLightIntensity(deltaTime);
	handleTimeUntilGameOver(deltaTime);

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
		if (m_factions[m_currentFactionTurn].m_controllerType == eControllerType::eAI)
		{
			handleAIMovementPhaseTimer(deltaTime);
		}			
	}
	else if (m_currentBattlePhase == BattlePhase::Attack)
	{
		updateAttackPhase();
		if (m_factions[m_currentFactionTurn].m_controllerType == eControllerType::eAI)
		{
			handleAIAttackPhaseTimer(deltaTime);
		}
	}
}

void Battle::moveFactionShipToPosition(ShipOnTile shipOnTile)
{
	assert(m_currentBattlePhase == BattlePhase::Movement);
	getFaction(shipOnTile.factionName).moveShipToPosition(m_map, shipOnTile.shipID);
}

void Battle::moveFactionShipToPosition(ShipOnTile shipOnTile, eDirection endDirection)
{
	assert(m_currentBattlePhase == BattlePhase::Movement);
	getFaction(shipOnTile.factionName).moveShipToPosition(m_map, shipOnTile.shipID, endDirection);
}

void Battle::clearFactionShipMovementArea(ShipOnTile shipOnTile)
{
	assert(m_currentBattlePhase == BattlePhase::Movement);
	getFaction(shipOnTile.factionName).clearShipMovementArea(shipOnTile.shipID);
}

void Battle::generateFactionShipMovementArea(ShipOnTile shipOnTile, sf::Vector2i destination, bool displayOnlyLastPosition)
{
	assert(m_currentBattlePhase == BattlePhase::Movement);
	getFaction(shipOnTile.factionName).generateShipMovementArea(m_map, shipOnTile.shipID, destination, displayOnlyLastPosition);
}

void Battle::rectifyFactionShipMovementArea(ShipOnTile shipOnTile)
{
	assert(m_currentBattlePhase == BattlePhase::Movement);
	getFaction(shipOnTile.factionName).rectifyShipMovementArea(shipOnTile.shipID);
}

void Battle::deployFactionShipAtPosition(sf::Vector2i startingPosition, eDirection startingDirection)
{
	assert(m_currentBattlePhase == BattlePhase::Deployment);

	//Inform remote clients on Deployment
	if (m_onlineGame)
	{
		ShipOnTile shipToDeploy;
		for (const auto& ship : m_factions[m_currentFactionTurn].getAllShips())
		{
			if (!ship.isDeployed())
			{
				shipToDeploy.factionName = ship.getFactionName();
				shipToDeploy.shipID = ship.getID();
				break;
			}
		}
		
		ServerMessage messageToSend(eMessageType::eDeployShipAtPosition, shipToDeploy.factionName);
		sf::Vector2i deployAtPosition = m_factions[m_currentFactionTurn].getShip(shipToDeploy.shipID).getCurrentPosition();
		messageToSend.shipActions.emplace_back(shipToDeploy.shipID, deployAtPosition.x, deployAtPosition.y);
		
		NetworkHandler::getInstance().sendServerMessage(messageToSend);
	}

	m_factions[m_currentFactionTurn].deployShipAtPosition(m_map, startingPosition, startingDirection);

	if (m_factions[m_currentFactionTurn].isAllShipsDeployed())
	{
		advanceToNextBattlePhase();
	}
}

void Battle::deployFactionShipAtPosition(const ServerMessage & receivedServerMessage)
{
	assert(m_currentBattlePhase == BattlePhase::Deployment);

	m_factions[m_currentFactionTurn].deployShipAtPosition(m_map, receivedServerMessage.shipActions.back().position, eDirection::eNorth);

	if (m_factions[m_currentFactionTurn].isAllShipsDeployed())
	{
		advanceToNextBattlePhase();
	}
}

void Battle::setShipDeploymentAtPosition(sf::Vector2i position, eDirection direction)
{
	assert(m_currentBattlePhase == BattlePhase::Deployment);
	m_factions[m_currentFactionTurn].setShipDeploymentAtPosition(position, direction);
}

void Battle::fireFactionShipAtPosition(ShipOnTile firingShip, const Tile& firingPosition, const std::vector<const Tile*>& targetArea)
{
	assert(m_currentBattlePhase == BattlePhase::Attack);
	assert(!getFactionShip(firingShip).isWeaponFired());

	m_factions[firingShip.factionName].m_ships[firingShip.shipID].fireWeapon();

	if (!firingPosition.isShipOnTile())
	{
		return;
	}

	const Ship& firingShipInPlay = getFactionShip(firingShip);
	const Ship& enemyShipInPlay = getFactionShip(firingPosition.m_shipOnTile);

	//Disallow attacking same team
	if (enemyShipInPlay.getFactionName() != getCurrentFaction().m_factionName && !enemyShipInPlay.isDead())
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

			getFaction(enemyShipInPlay.getFactionName()).shipTakeDamage(enemyShipInPlay.getID(), firingShipInPlay.getDamage());
		}
	}
}

void Battle::advanceToNextBattlePhase()
{
	//Send server message
	if (m_currentBattlePhase == BattlePhase::Deployment)
	{
		bool allPlayersDeployed = true;
		bool allHumansDeployed = true;
		if (m_currentDeploymentState == eDeploymentState::DeployingPlayer)
		{
			for (int i = 0; i < m_factions.size(); ++i)
			{
				if (!m_factions[i].isActive())
				{
					continue;
				}

				if (m_factions[i].m_controllerType != eControllerType::eLocalPlayer &&
					m_factions[i].m_controllerType != eControllerType::eRemotePlayer)
				{
					continue;
				}

				//If first ship hasn't been deployed - rest haven't
				//Proceed to deploy this human player ships
				if (!m_factions[i].m_ships[0].isDeployed())
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
				if(!m_factions[i].isActive())
				{
					continue;
				}
				for (int i = 0; i < m_factions.size(); ++i)
				{
					if (!m_factions[i].isActive())
					{
						continue;
					}
				}

				if (m_factions[i].m_controllerType != eControllerType::eAI)
				{
					continue;
				}

				//If first ship hasn't been deployed - rest haven't
				//Proceed to deploy this human player ships
				if (!m_factions[i].m_ships[0].isDeployed())
				{
					m_currentFactionTurn = i;
					AI::handleDeploymentPhase(*this, m_factions[m_currentFactionTurn]);
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
				if (faction.isActive())
				{
					faction.clearSpawnArea();
				}
			}

			for (auto& ship : m_factions[m_currentFactionTurn].m_ships)
			{
				ship.enableAction();
			}

			if (m_factions[m_currentFactionTurn].m_controllerType == eControllerType::eAI)
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

		for (auto& entity : m_factions[m_currentFactionTurn].m_ships)
		{
			entity.enableAction();
		}

		if (!m_factions[m_currentFactionTurn].isEliminated() && m_factions[m_currentFactionTurn].m_controllerType == eControllerType::eAI)
		{
			m_timeUntilAITurn.setActive(true);
			//TODO: Refactor
			//GameEventMessenger::getInstance().broadcast(GameEvent::eEnteredAITurn);
		}
	}
	else if (m_currentBattlePhase == BattlePhase::Attack)
	{
		switchToBattlePhase(BattlePhase::Movement);

		for (auto& entity : m_factions[m_currentFactionTurn].m_ships)
		{
			entity.disableAction();
		}

		updateWindDirection();
		incrementFactionTurn();
		for (auto& entity : m_factions[m_currentFactionTurn].m_ships)
		{
			entity.enableAction();
		}

		if (!m_factions[m_currentFactionTurn].isEliminated() && m_factions[m_currentFactionTurn].m_controllerType == eControllerType::eAI)
		{
			m_timeUntilAITurn.setActive(true);
			//TODO: Refactor		
			//GameEventMessenger::getInstance().broadcast(GameEvent::eEnteredAITurn);
		}
		else if (m_factions[m_currentFactionTurn].m_controllerType == eControllerType::eLocalPlayer)
		{
			//TODO: Refactor
			//GameEventMessenger::getInstance().broadcast(GameEvent::eLeftAITurn);
		}
	}
}

void Battle::switchToBattlePhase(BattlePhase newBattlePhase)
{
	m_currentBattlePhase = newBattlePhase;
	GameEventMessenger::getInstance().broadcast(GameEvent(), eGameEvent::eEnteredNewBattlePhase);
}

std::vector<FactionName> Battle::getAllFactionsInPlay() const
{
	std::vector<FactionName> allFactionsInPlay;
	allFactionsInPlay.reserve(m_factions.size());
	for (const auto& faction : m_factions)
	{
		if (faction.isActive())
		{
			allFactionsInPlay.push_back(faction.m_factionName);
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
	for (auto& ship : m_factions[m_currentFactionTurn].m_ships)
	{
		ship.update(deltaTime);
	}
	for (const auto& entity : m_factions[m_currentFactionTurn].m_ships)
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
	for (const auto& ship : m_factions[m_currentFactionTurn].m_ships)
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

Faction& Battle::getCurrentPlayer()
{
	assert(m_factions[m_currentFactionTurn].isActive());
	return m_factions[m_currentFactionTurn];
}

Faction & Battle::getFaction(FactionName factionName)
{
	assert(m_factions[static_cast<int>(factionName)].isActive());
	return m_factions[static_cast<int>(factionName)];
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
			if (m_factions[i].isActive())
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
			if (m_factions[i].isActive())
			{
				m_currentFactionTurn = i;
				break;
			}
		}
	}
}

bool Battle::isRunning() const
{
	return m_isRunning;
}

bool Battle::isShipBelongToFactionInPlay(ShipOnTile shipOnTile) const
{
	assert(m_factions[m_currentFactionTurn].isActive());
	return shipOnTile.factionName == getCurrentFaction().m_factionName;
}

const Map & Battle::getMap() const
{
	return m_map;
}

BattlePhase Battle::getCurrentBattlePhase() const
{
	return m_currentBattlePhase;
}

const Faction& Battle::getCurrentFaction() const
{
	assert(m_factions[m_currentFactionTurn].isActive());
	return m_factions[m_currentFactionTurn];
}

eControllerType Battle::getCurrentPlayerType() const
{
	assert(m_factions[m_currentFactionTurn].isActive());
	return m_factions[m_currentFactionTurn].m_controllerType;
}

const Ship & Battle::getFactionShip(ShipOnTile shipOnTile) const
{
	assert(shipOnTile.isValid());
	return m_factions[static_cast<int>(shipOnTile.factionName)].getShip(shipOnTile.shipID);
}

const std::vector<Ship>& Battle::getCurrentFactionShips() const
{
	assert(m_factions[m_currentFactionTurn].isActive());
	return m_factions[m_currentFactionTurn].getAllShips();
}

const Faction& Battle::getFaction(FactionName factionName) const
{
	assert(m_factions[static_cast<int>(factionName)].isActive());
	return m_factions[static_cast<int>(factionName)];
}

void Battle::receiveServerMessage(const ServerMessage& serverMessage)
{
	switch (serverMessage.type)
	{
	case eMessageType::eDeployShipAtPosition:
		deployFactionShipAtPosition(serverMessage);
		break;

	case eMessageType::eMoveShipToPosition :
	{
		break;
	}
	case eMessageType::eAttackShipAtPosition:
		break;
	}
}

void Battle::onEndBattlePhaseEarly(GameEvent gameEvent)
{
	if (m_currentBattlePhase == BattlePhase::Movement)
	{
		auto& currentFactionShips = m_factions[m_currentFactionTurn].m_ships;
		auto cIter = std::find_if(currentFactionShips.cbegin(), currentFactionShips.cend(), [](const auto& ship) { return ship.isMovingToDestination(); });
		if (cIter == currentFactionShips.cend())
		{
			advanceToNextBattlePhase();
		}
	}
	else if (m_currentBattlePhase == BattlePhase::Attack)
	{
		advanceToNextBattlePhase();
	}
}

void Battle::onFactionShipDestroyed(GameEvent gameEvent)
{
	auto factionShipDestroyedEvent = static_cast<const FactionShipDestroyedEvent*>(gameEvent.data);
	assert(m_factions[static_cast<int>(factionShipDestroyedEvent->factionName)].getShip(factionShipDestroyedEvent->shipID).isDead());
	
	int factionsWithShipsRemaining = 0;
	for (const auto& faction : m_factions)
	{
		if (faction.isActive() && !faction.isEliminated())
		{
			++factionsWithShipsRemaining;
		}
	}
	assert(factionsWithShipsRemaining > 0);
	//Found winning faction
	if (factionsWithShipsRemaining == 1)
	{
		for (const auto& faction : m_factions)
		{
			if (faction.isActive() && !faction.isEliminated())
			{
				std::cout << "Winning Faction : " << static_cast<int>(faction.m_factionName) << "\n";
				m_timeUntilGameOver.setActive(true);
				break;
			}
		}
	}
}