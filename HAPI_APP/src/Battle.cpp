#include "Battle.h"
#include "Utilities/XMLParser.h"
#include "GameEventMessenger.h"
#include "AIHandler.h"
#include "Textures.h"
#include "NetworkHandler.h"
#include <iostream>
#include "Utilities/Utilities.h"
#include "PathFinding.h"

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
		for (auto& ship : m_factions[m_currentFactionTurn].m_ships)
		{
			if (!ship.isDead() &&
				!ship.isDestinationSet())
			{
				AIHandler::getInstance().handleMovementPhase(*this, m_map, m_factions[m_currentFactionTurn], i);
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
				AIHandler::getInstance().handleShootingPhase(*this, m_map, m_factions[m_currentFactionTurn], i);
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
	m_map(),
	m_currentBattlePhase(BattlePhase::Deployment),
	m_currentDeploymentState(eDeploymentState::NotStarted),
	m_battleUI(*this),
	m_explosionParticles(),
	m_fireParticles(),
	m_timeUntilAITurn(1.5f, false),
	m_timeBetweenAIUnits(0.3f, false),
	m_timeUntilGameOver(2.f, false),
	m_isRunning(true),
	m_onlineGame(false),
	m_currentFactionTurn(0)
{
	m_explosionParticles.reserve(MAX_PARTICLES);
	m_fireParticles.reserve(MAX_PARTICLES);
	for (int i = 0; i < MAX_PARTICLES; i++)
	{
		m_explosionParticles.emplace_back(0.10, *Textures::getInstance().m_explosionParticles, 2.5f);
		m_fireParticles.emplace_back(0.05, *Textures::getInstance().m_fireParticles, 2.0f);
	}

	GameEventMessenger::getInstance().subscribe(std::bind(&Battle::onFactionShipDestroyed, this, std::placeholders::_1), eGameEvent::eFactionShipDestroyed);
}

Battle::~Battle()
{
	GameEventMessenger::getInstance().unsubscribe(eGameEvent::eFactionShipDestroyed);
}

void Battle::quitGame()
{
	m_isRunning = false;
}

void Battle::startOnlineGame(const std::string & newMapName, const std::vector<ServerMessageSpawnPosition>& factionSpawnPositions)
{
	m_onlineGame = true;
	m_map.loadmap(newMapName);
	m_battleUI.setMaxCameraOffset(m_map.getDimensions());
	PathFinding::getInstance().loadTileData(m_map);
	
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

	//Set initial deployment state
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
	m_onlineGame = false;
	m_map.loadmap(levelName);
	m_battleUI.setMaxCameraOffset(m_map.getDimensions());
	PathFinding::getInstance().loadTileData(m_map);

	for (auto& faction : m_factions)
	{
		if (faction.isActive())
		{
			faction.createSpawnArea(m_map, m_map.getRandomSpawnPosition());
		}
	}

	bool playerFound = false;
	for (const auto& faction : m_factions)
	{
		if (faction.isActive() && faction.m_controllerType == eControllerType::eLocalPlayer)
		{
			playerFound = true;
			break;
		}
	}

	//Set initial deployment state
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
			faction.render(window, m_map, m_currentBattlePhase, m_currentFactionTurn);
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

void Battle::renderFactionShipsMovementGraph(sf::RenderWindow & window)
{
	if (m_currentBattlePhase == BattlePhase::Movement)
	{
		assert(m_factions[m_currentFactionTurn].isActive());
		return m_factions[m_currentFactionTurn].renderShipsMovementGraphs(window, m_map);
	}
}

void Battle::handleInput(const sf::RenderWindow& window, const sf::Event & currentEvent)
{
	sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
	mousePosition += MOUSE_POSITION_OFFSET;

	if (currentEvent.type == sf::Event::MouseMoved)
	{
		m_battleUI.moveCamera(window.getSize(), mousePosition);
	}

	if (m_factions[m_currentFactionTurn].m_controllerType != eControllerType::eRemotePlayer)
	{
		m_battleUI.handleInput(currentEvent, mousePosition);
	}
}

void Battle::update(float deltaTime)
{
	m_battleUI.update(deltaTime);
	m_map.setDrawOffset(m_battleUI.getCameraPosition());
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
	assert(m_factions[m_currentFactionTurn].m_factionName == shipOnTile.factionName);

	if (getFactionShip(shipOnTile).getMovementArea().empty())
	{
		return;
	}

	getFaction(shipOnTile.factionName).moveShipToPosition(m_map, shipOnTile.shipID);

	if (m_onlineGame && m_factions[m_currentFactionTurn].m_controllerType != eControllerType::eAI)
	{
		sf::Vector2i destination = getFaction(shipOnTile.factionName).getShip(shipOnTile.shipID).getMovementArea().back().pair();
		eDirection endDirection = getFaction(shipOnTile.factionName).getShip(shipOnTile.shipID).getMovementArea().back().dir;

		ServerMessage messageToSend(eMessageType::eMoveShipToPosition, shipOnTile.factionName);
		messageToSend.shipActions.emplace_back(shipOnTile.shipID, destination.x, destination.y, endDirection);
		NetworkHandler::getInstance().sendServerMessage(messageToSend);
	}
}

void Battle::moveFactionShipToPosition(ShipOnTile shipOnTile, eDirection endDirection)
{
	assert(m_currentBattlePhase == BattlePhase::Movement);
	assert(m_factions[m_currentFactionTurn].m_factionName == shipOnTile.factionName);

	if (getFactionShip(shipOnTile).getMovementArea().empty())
	{
		return;
	}

	getFaction(shipOnTile.factionName).moveShipToPosition(m_map, shipOnTile.shipID, endDirection);

	if (m_onlineGame && m_factions[m_currentFactionTurn].m_controllerType != eControllerType::eAI)
	{
		sf::Vector2i destination = getFaction(shipOnTile.factionName).getShip(shipOnTile.shipID).getMovementArea().back().pair();

		ServerMessage messageToSend(eMessageType::eMoveShipToPosition, shipOnTile.factionName);
		messageToSend.shipActions.emplace_back(shipOnTile.shipID, destination.x, destination.y, endDirection);
		NetworkHandler::getInstance().sendServerMessage(messageToSend);
	}
}

void Battle::moveFactionShipsToPosition(ShipSelector & selectedShips)
{
	ServerMessage messageToSend;
	int selectedShipsSize = selectedShips.getSelectedShips().size();
	for (int i = 0; i < selectedShipsSize; ++i)
	{
		ShipOnTile selectedShip = selectedShips.removeSelectedShip();
		if (getFaction(selectedShip.factionName).getShip(selectedShip.shipID).getMovementArea().empty())
		{
			continue;
		}

		getFaction(selectedShip.factionName).moveShipToPosition(m_map, selectedShip.shipID);

		if (m_onlineGame && m_factions[m_currentFactionTurn].m_controllerType != eControllerType::eAI)
		{
			sf::Vector2i destination = getFaction(selectedShip.factionName).getShip(selectedShip.shipID).getMovementArea().back().pair();
			eDirection endDirection = getFaction(selectedShip.factionName).getShip(selectedShip.shipID).getMovementArea().back().dir;

			messageToSend.type = eMessageType::eMoveShipToPosition;
			messageToSend.faction = selectedShip.factionName;
			messageToSend.shipActions.emplace_back(selectedShip.shipID, destination.x, destination.y, endDirection);
			NetworkHandler::getInstance().sendServerMessage(messageToSend);
		}
	}

	assert(messageToSend.shipActions.size() == selectedShipsSize);
}

void Battle::generateFactionShipsMovementArea(const std::vector<const Tile*>& movementArea, const ShipSelector & shipSelector)
{
	//Generate ship movement paths to positions
	int shipIndex = 0;
	for (const auto& tile : movementArea)
	{
		if (tile && !tile->isShipOnTile())
		{
			for (int i = shipIndex; i < shipSelector.getSelectedShips().size();)
			{
				ShipOnTile selectedShip = shipSelector.getSelectedShips()[i].m_shipOnTile;
				generateFactionShipMovementArea(selectedShip, tile->m_tileCoordinate, true);

				++i;
				shipIndex = i;
				break;
			}
		}
	}

	//Fix ship movement graphs after generation
	for (SelectedShip selectedShip : shipSelector.getSelectedShips())
	{
		getFaction(selectedShip.m_shipOnTile.factionName).rectifyShipMovementArea(selectedShip.m_shipOnTile.shipID);
	}
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

void Battle::deployFactionShipAtPosition(sf::Vector2i startingPosition, eDirection startingDirection)
{
	assert(m_currentBattlePhase == BattlePhase::Deployment);

	m_factions[m_currentFactionTurn].deployShipAtPosition(m_map, startingPosition, startingDirection);

	//Inform remote clients on Deployment
	if (m_onlineGame && m_factions[m_currentFactionTurn].m_controllerType != eControllerType::eAI)
	{
		ServerMessage messageToSend(eMessageType::eDeployShipAtPosition, m_factions[m_currentFactionTurn].m_factionName);
		messageToSend.shipActions.emplace_back(startingPosition.x, startingPosition.y, startingDirection);
		NetworkHandler::getInstance().sendServerMessage(messageToSend);
	}

	if (m_factions[m_currentFactionTurn].isAllShipsDeployed())
	{
		advanceToNextBattlePhase();
	}
}

void Battle::deployFactionShipAtPosition(const ServerMessage & receivedServerMessage)
{
	assert(m_currentBattlePhase == BattlePhase::Deployment);

	m_factions[m_currentFactionTurn].deployShipAtPosition(m_map, receivedServerMessage.shipActions.back().position, 
		receivedServerMessage.shipActions.back().direction);

	if (m_factions[m_currentFactionTurn].isAllShipsDeployed())
	{
		advanceToNextBattlePhase();
	}
}

void Battle::moveFactionShipToPosition(const ServerMessage & receivedServerMessage)
{
	assert(m_currentBattlePhase == BattlePhase::Movement);
	for (const auto& shipAction : receivedServerMessage.shipActions)
	{
		ShipOnTile shipToMove(receivedServerMessage.faction, shipAction.shipID);
		generateFactionShipMovementArea(shipToMove, shipAction.position, true);
		m_factions[m_currentFactionTurn].rectifyShipMovementArea(shipToMove.shipID);
		
	}

	for (const auto& shipAction : receivedServerMessage.shipActions)
	{
		ShipOnTile shipToMove(receivedServerMessage.faction, shipAction.shipID);
		getFaction(shipToMove.factionName).moveShipToPosition(m_map, shipToMove.shipID, shipAction.direction);
	}
}

void Battle::fireFactionShipAtPosition(const ServerMessage & receivedServerMessage)
{
	assert(m_currentBattlePhase == BattlePhase::Attack);

	for (const auto& shipAction : receivedServerMessage.shipActions)
	{
		TileArea& targetArea = m_battleUI.getTargetArea();
		targetArea.clearTileArea();

		const Ship& firingShip = getCurrentFaction().getShip(shipAction.shipID);
		switch (firingShip .getShipType())
		{
		case eShipType::eFrigate:
			m_map.getTileCone(targetArea.m_tileArea, firingShip.getCurrentPosition(),
				firingShip.getRange(),
				firingShip.getCurrentDirection(), true);
			break;

		case eShipType::eSniper:
			m_map.getTileLine(targetArea.m_tileArea, firingShip.getCurrentPosition(),
				firingShip.getRange(),
				firingShip.getCurrentDirection(), true);
			break;

		case eShipType::eTurtle:
			m_map.getTileRadius(targetArea.m_tileArea, firingShip.getCurrentPosition(),
				firingShip.getRange(), true);
			break;

		case eShipType::eFire:
			m_map.getTileLine(targetArea.m_tileArea, firingShip.getCurrentPosition(),
				firingShip.getRange(), Utilities::getOppositeDirection(firingShip.getCurrentDirection()), true);
			break;
		}

		m_factions[receivedServerMessage.faction].m_ships[shipAction.shipID].fireWeapon();

		ShipOnTile shipOnFiringPosition = m_map.getTile(shipAction.position)->m_shipOnTile;
		if (!shipOnFiringPosition.isValid())
		{
			return;
		}

		const Ship& enemyShip = getFaction(shipOnFiringPosition.factionName).getShip(shipOnFiringPosition.shipID);
		//Disallow attacking same team
		if (enemyShip.getFactionName() != getCurrentFaction().m_factionName && !enemyShip.isDead())
		{
			//Find Enemy Ship 
			sf::Vector2i enemyShipInPlayPosition = enemyShip.getCurrentPosition();
			auto cIter = std::find_if(targetArea.m_tileArea.cbegin(), targetArea.m_tileArea.cend(),
				[enemyShipInPlayPosition](const auto& tile) { return enemyShipInPlayPosition == tile->m_tileCoordinate; });
			//Enemy Ship within range of weapon
			if (cIter != targetArea.m_tileArea.cend())
			{
				//const Ship& firingShipInPlay = getFactionShip(firingShip);
				if (firingShip.getShipType() == eShipType::eFire)
				{
					playFireAnimation(firingShip.getCurrentDirection(), firingShip.getCurrentPosition());
				}
				else
				{
					playExplosionAnimation(enemyShip.getCurrentPosition());
				}

				getFaction(enemyShip.getFactionName()).shipTakeDamage(enemyShip.getID(), firingShip.getDamage());
			}
		}
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
	assert(m_factions[m_currentFactionTurn].m_factionName == firingShip.factionName);
	assert(!getFactionShip(firingShip).isWeaponFired());

	if (m_onlineGame && m_factions[m_currentFactionTurn].m_controllerType != eControllerType::eAI)
	{
		ServerMessage messageToSend(eMessageType::eAttackShipAtPosition, firingShip.factionName);
		messageToSend.shipActions.emplace_back(firingShip.shipID,
			firingPosition.m_tileCoordinate.x, firingPosition.m_tileCoordinate.y);

		NetworkHandler::getInstance().sendServerMessage(messageToSend);
	}

	m_factions[firingShip.factionName].m_ships[firingShip.shipID].fireWeapon();

	if (!firingPosition.isShipOnTile())
	{
		return;
	}

	//Disallow attacking same team
	const Ship& enemyShipInPlay = getFactionShip(firingPosition.m_shipOnTile);
	if (enemyShipInPlay.getFactionName() != getCurrentFaction().m_factionName && !enemyShipInPlay.isDead())
	{
		//Find Enemy Ship 
		sf::Vector2i enemyShipInPlayPosition = enemyShipInPlay.getCurrentPosition();
		auto cIter = std::find_if(targetArea.cbegin(), targetArea.cend(),
			[enemyShipInPlayPosition](const auto& tile) { return enemyShipInPlayPosition == tile->m_tileCoordinate; });
		//Enemy Ship within range of weapon
		if (cIter != targetArea.cend())
		{
			const Ship& firingShipInPlay = getFactionShip(firingShip);
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
	if (m_currentBattlePhase == BattlePhase::Deployment)
	{
		bool allFactionsDeployed = true;
		bool allPlayersDeployed = true;
		if (m_currentDeploymentState == eDeploymentState::DeployingPlayer)
		{
			for (int i = 0; i < m_factions.size(); ++i)
			{
				if (!m_factions[i].isActive() || m_factions[i].m_controllerType == eControllerType::eAI)
				{
					continue;
				}

				//Faction hasn't been deployed - begin to deploy faction
				if (!m_factions[i].m_ships[0].isDeployed())
				{
					m_currentFactionTurn = i;
					allFactionsDeployed = false;
					allPlayersDeployed = false;
					break;
				}
			}

			if (allPlayersDeployed)
			{
				m_currentDeploymentState = eDeploymentState::DeployingAI;
			}
		}
		
		if (m_currentDeploymentState == eDeploymentState::DeployingAI)
		{
			for (int i = 0; i < m_factions.size(); ++i)
			{
				if(!m_factions[i].isActive() || m_factions[i].m_controllerType != eControllerType::eAI)
				{
					continue;
				}

				//Faction hasn't been deployed - begin to deploy faction
				if (!m_factions[i].m_ships[0].isDeployed())
				{
					m_currentFactionTurn = i;
					AIHandler::getInstance().handleDeploymentPhase(*this, m_factions[m_currentFactionTurn]);
					allFactionsDeployed = false;
					break;
				}
			}
		}

		if (allFactionsDeployed)
		{
			switchToBattlePhase(BattlePhase::Movement);
			m_currentDeploymentState = eDeploymentState::Finished;
			m_currentFactionTurn = 0;
			for (auto& ship : m_factions[m_currentFactionTurn].m_ships)
			{
				ship.enableAction();
			}

			if (m_factions[m_currentFactionTurn].m_controllerType == eControllerType::eAI)
			{
				m_timeUntilAITurn.setActive(true);
			}

			//Clear all faction spawn areas
			for (auto& faction : m_factions)
			{
				if (faction.isActive())
				{
					faction.clearSpawnArea();
				}
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

	assert(m_currentFactionTurn < static_cast<int>(m_factions.size()));

	////Select next available faction
	//do
	//{
	//	m_currentFactionTurn = (m_currentFactionTurn + 1) & static_cast<int>(m_factions.size());
	//} while (!m_factions[m_currentFactionTurn].isActive());

	bool nextFactionSelected = false;
	while (!nextFactionSelected)
	{
		//Start from beginning
		if (m_currentFactionTurn == static_cast<int>(m_factions.size()) - 1)
		{
			for (int i = 0; i < m_factions.size(); ++i)
			{
				if (m_factions[i].isActive())
				{
					m_currentFactionTurn = i;
					nextFactionSelected = true;
					break;
				}
			}
		}
		//Search subsequent factions
		else if (m_currentFactionTurn < static_cast<int>(m_factions.size()) - 1)
		{
			++m_currentFactionTurn;
			if (m_factions[m_currentFactionTurn].isActive())
			{
				nextFactionSelected = true;
			}
		}
	}
}


bool Battle::isRunning() const
{
	return m_isRunning;
}

bool Battle::isShipBelongToCurrentFaction(ShipOnTile shipOnTile) const
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

const Ship & Battle::getFactionShip(ShipOnTile shipOnTile) const
{
	assert(shipOnTile.isValid());
	return m_factions[static_cast<int>(shipOnTile.factionName)].getShip(shipOnTile.shipID);
}

const Faction& Battle::getFaction(FactionName factionName) const
{
	assert(m_factions[static_cast<int>(factionName)].isActive());
	return m_factions[static_cast<int>(factionName)];
}

void Battle::receiveServerMessage(const ServerMessage& receivedServerMessage)
{
	switch (receivedServerMessage.type)
	{
	case eMessageType::eDeployShipAtPosition :
		assert(receivedServerMessage.faction == m_factions[m_currentFactionTurn].m_factionName);
		deployFactionShipAtPosition(receivedServerMessage);
		break;

	case eMessageType::eMoveShipToPosition :
		assert(receivedServerMessage.faction == m_factions[m_currentFactionTurn].m_factionName);
		moveFactionShipToPosition(receivedServerMessage);
		break;

	case eMessageType::eAttackShipAtPosition :
		assert(receivedServerMessage.faction == m_factions[m_currentFactionTurn].m_factionName);
		fireFactionShipAtPosition(receivedServerMessage);
		break;

	case eMessageType::eClientDisconnected:
		m_factions[static_cast<int>(receivedServerMessage.faction)].m_controllerType = eControllerType::eAI;
		if (m_factions[m_currentFactionTurn].m_factionName == receivedServerMessage.faction)
		{
			m_timeUntilAITurn.setActive(true);
		}
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

	//Found winning faction
	assert(factionsWithShipsRemaining > 0);
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