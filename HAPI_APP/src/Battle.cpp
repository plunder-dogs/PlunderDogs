#include "Battle.h"
#include "Utilities/MapParser.h"
#include "GameEventMessenger.h"
#include "AI.h"

using namespace HAPISPACE;
constexpr float DRAW_ENTITY_OFFSET_X{ 16 };
constexpr float DRAW_ENTITY_OFFSET_Y{ 32 };

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

void Battle::Particle::run(float deltaTime, const Map& map)
{
	if (m_isEmitting)
	{
		const std::pair<int, int> tileTransform = map.getTileScreenPos(m_position);
		m_particle->GetTransformComp().SetPosition({
			tileTransform.first + DRAW_ENTITY_OFFSET_X * map.getDrawScale(),
			tileTransform.second + DRAW_ENTITY_OFFSET_Y * map.getDrawScale() });

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
		for (auto& ship : m_players[m_currentPlayerTurn]->m_ships)
		{
			if (!ship.isDead() &&
				!ship.isDestinationSet())
			{
				AI::handleMovementPhase(*this, m_map, m_players[m_currentPlayerTurn], i);
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
		for (auto& ship : m_players[m_currentPlayerTurn]->m_ships)
		{
			if (!ship.isDead() && !ship.isWeaponFired())
			{
				AI::handleShootingPhase(*this, m_map, m_players[m_currentPlayerTurn], i);
				m_timeBetweenAIUnits.reset();
				return;
			}
			i++;
		}
		m_timeBetweenAIUnits.setActive(false);
		m_timeBetweenAIUnits.reset();
	}
}

Battle::Battle(std::vector<std::unique_ptr<Player>>& players)
	: m_players(players),
	m_currentPlayerTurn(0),
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
	assert(!m_players.empty());

	m_map.loadmap(newMapName);
	m_battleUI.loadGUI(m_map.getDimensions());
	
	//Assign Spawn Position
	for (auto& player : m_players)
	{
		player->createSpawnArea(m_map);
	}

	//Set initial deployment state
	bool humanPlayerFound = false;
	for (const auto& player : m_players)
	{
		if (player->m_playerType == ePlayerType::eHuman)
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

	for (const auto& player : m_players)
	{
		player->render(m_map);
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
			explosionParticle.run(deltaTime, m_map);
		}
		for (auto& fireParticle : m_fireParticles)
		{
			fireParticle.run(deltaTime, m_map);
		}

		if (m_currentBattlePhase == BattlePhase::Movement)
		{
			updateMovementPhase(deltaTime);
			if (m_players[m_currentPlayerTurn]->m_playerType == ePlayerType::eAI)
			{
				handleAIMovementPhaseTimer(deltaTime);
			}			
		}
		else if (m_currentBattlePhase == BattlePhase::Attack)
		{
			updateAttackPhase();
			if (m_players[m_currentPlayerTurn]->m_playerType == ePlayerType::eAI)
			{
				handleAIAttackPhaseTimer(deltaTime);
			}
		}
	}

	m_winningFactionHandler.update(deltaTime);
}

void Battle::moveEntityToPosition(Ship& entity, const Tile& destination)
{
	assert(m_currentBattlePhase == BattlePhase::Movement);
	entity.moveEntity(m_map, destination);
}

void Battle::moveEntityToPosition(Ship& entity, const Tile& destination, eDirection endDirection)
{
	assert(m_currentBattlePhase == BattlePhase::Movement);
	entity.moveEntity(m_map, destination, endDirection);
}

void Battle::deployShipAtPosition(std::pair<int, int> startingPosition, eDirection startingDirection)
{
	assert(m_currentBattlePhase == BattlePhase::Deployment);
	assert(m_players[m_currentPlayerTurn]->m_shipToDeploy);

	m_players[m_currentPlayerTurn]->m_shipToDeploy->deployAtPosition(startingPosition, *this, startingDirection);
	m_map.assignTileToShip(*m_players[m_currentPlayerTurn]->m_shipToDeploy);
	
	bool allShipsDeployed = true;
	for (auto& ship : m_players[m_currentPlayerTurn]->m_ships)
	{
		if (!ship.isDeployed())
		{
			m_players[m_currentPlayerTurn]->m_shipToDeploy = &ship;
			allShipsDeployed = false;
		}
	}

	if (allShipsDeployed)
	{
		nextTurn();
	}
}

bool Battle::setShipDeploymentAtPosition(std::pair<int, int> position)
{
	assert(m_currentBattlePhase == BattlePhase::Deployment);
	auto& currentPlayer = getCurrentPlayer();
	assert(getCurrentPlayer()->m_shipToDeploy);

	auto iter = std::find_if(currentPlayer->m_spawnArea.cbegin(), currentPlayer->m_spawnArea.cend(), 
		[position](const auto& spawnArea) { return position == spawnArea.m_position; });
	if (iter != currentPlayer->m_spawnArea.cend())
	{
		currentPlayer->m_shipToDeploy->setDeploymentPosition(position, *this);
		return true;
	}
	else
	{
		currentPlayer->m_shipToDeploy->setDeploymentPosition(position, *this);
		return false;
	}
}

bool Battle::fireEntityWeaponAtPosition(const Tile& tileOnPlayer, const Tile& tileOnAttackPosition, const std::vector<const Tile*>& targetArea)
{
	assert(m_currentBattlePhase == BattlePhase::Attack);
	assert(tileOnPlayer.m_shipOnTile);

	if (!tileOnPlayer.m_shipOnTile)
	{
		return false;
	}
	assert(!tileOnPlayer.m_shipOnTile->isWeaponFired());

	//Disallow attacking same team
	if (tileOnAttackPosition.m_shipOnTile && tileOnAttackPosition.m_shipOnTile->getFactionName() != getCurrentFaction()
			&& !tileOnAttackPosition.m_shipOnTile->isDead())
	{
		//Find entity 
		auto tileCoordinate = tileOnAttackPosition.m_shipOnTile->getCurrentPosition();
		auto cIter = std::find_if(targetArea.cbegin(), targetArea.cend(), [tileCoordinate](const auto& tile) { if(tile) return tileCoordinate == tile->m_tileCoordinate; });
		//Enemy within range of weapon
		if (cIter != targetArea.cend())
		{
			if (tileOnPlayer.m_shipOnTile->getShipType() == eShipType::eFire)
			{
				playFireAnimation(*tileOnPlayer.m_shipOnTile, targetArea[0]->m_tileCoordinate);
			}
			else
			{
				playExplosionAnimation(*tileOnAttackPosition.m_shipOnTile);
			}

			tileOnPlayer.m_shipOnTile->fireWeapon();
			auto& enemy = tileOnAttackPosition.m_shipOnTile;
			enemy->takeDamage(tileOnPlayer.m_shipOnTile->getDamage(), enemy->getFactionName());
			
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
			for (int i = 0; i < m_players.size(); ++i)
			{
				if (m_players[i]->m_playerType != ePlayerType::eHuman)
				{
					continue;
				}

				//If first ship hasn't been deployed - rest haven't
				//Proceed to deploy this human player ships
				if (!m_players[i]->m_ships[0].isDeployed())
				{
					m_currentPlayerTurn = i;
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
			for (int i = 0; i < m_players.size(); ++i)
			{
				if (m_players[i]->m_playerType != ePlayerType::eAI)
				{
					continue;
				}

				//If first ship hasn't been deployed - rest haven't
				//Proceed to deploy this human player ships
				if (!m_players[i]->m_ships[0].isDeployed())
				{
					m_currentPlayerTurn = i;
					AI::handleDeploymentPhase(*this, m_map, *m_players[m_currentPlayerTurn].get());
					allPlayersDeployed = false;
					break;
				}
			}
		}

		if (allPlayersDeployed)
		{
			m_currentBattlePhase = BattlePhase::Movement;
			m_currentPlayerTurn = 0;
			GameEventMessenger::getInstance().broadcast(GameEvent::eNewTurn);
			for (auto& ship : m_players[m_currentPlayerTurn]->m_ships)
			{
				ship.enableAction();
			}

			for (auto& player : m_players)
			{
				player->m_spawnArea.clear();
			}

			if (m_players[m_currentPlayerTurn]->m_playerType == ePlayerType::eAI)
			{
				m_timeUntilAITurn.setActive(true);
				GameEventMessenger::getInstance().broadcast(GameEvent::eEnteredAITurn);
			}
		}
	}
	else if (m_currentBattlePhase == BattlePhase::Movement)
	{
		m_currentBattlePhase = BattlePhase::Attack;
		GameEventMessenger::getInstance().broadcast(GameEvent::eNewTurn);
		GameEventMessenger::getInstance().broadcast(GameEvent::eEnteringAttackPhase);

		for (auto& entity : m_players[m_currentPlayerTurn]->m_ships)
		{
			entity.enableAction();
		}

		if (!m_players[m_currentPlayerTurn]->m_eliminated && m_players[m_currentPlayerTurn]->m_playerType == ePlayerType::eAI)
		{
			m_timeUntilAITurn.setActive(true);
			GameEventMessenger::getInstance().broadcast(GameEvent::eEnteredAITurn);
		}
	}
	else if (m_currentBattlePhase == BattlePhase::Attack)
	{
		m_currentBattlePhase = BattlePhase::Movement;
		GameEventMessenger::getInstance().broadcast(GameEvent::eNewTurn);
		GameEventMessenger::getInstance().broadcast(GameEvent::eEnteringMovementPhase);

		for (auto& entity : m_players[m_currentPlayerTurn]->m_ships)
		{
			entity.disableAction();
		}

		updateWindDirection();
		incrementPlayerTurn();
		for (auto& entity : m_players[m_currentPlayerTurn]->m_ships)
		{
			entity.enableAction();
		}

		if (!m_players[m_currentPlayerTurn]->m_eliminated && m_players[m_currentPlayerTurn]->m_playerType == ePlayerType::eAI)
		{
			m_timeUntilAITurn.setActive(true);
			GameEventMessenger::getInstance().broadcast(GameEvent::eEnteredAITurn);
		}
		else if (m_players[m_currentPlayerTurn]->m_playerType == ePlayerType::eHuman)
		{
			GameEventMessenger::getInstance().broadcast(GameEvent::eLeftAITurn);
		}
	}
}

std::vector<FactionName> Battle::getAllFactionsInPlay() const
{
	std::vector<FactionName> allFactionsInPlay;
	allFactionsInPlay.reserve(m_players.size());
	for (const auto& player : m_players)
	{
		allFactionsInPlay.push_back(player->m_factionName);
	}

	assert(!allFactionsInPlay.empty());
	return allFactionsInPlay;
}

void Battle::playFireAnimation(Ship& entity, std::pair<int, int> position)
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

void Battle::playExplosionAnimation(Ship& entity)
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
	for (auto& entity : m_players[m_currentPlayerTurn]->m_ships)
	{
		if (entity.isDead())
			continue;
		entity.update(deltaTime, m_map);
	}
	for (auto& entity : m_players[m_currentPlayerTurn]->m_ships)
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
	for (const auto& ship : m_players[m_currentPlayerTurn]->m_ships)
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

Player& Battle::getPlayer(FactionName factionName)
{
	auto cIter = std::find_if(m_players.begin(), m_players.end(), [factionName](auto& player) { return factionName == player->m_factionName; });
	assert(cIter != m_players.end());
	return *cIter->get();
}

std::unique_ptr<Player>& Battle::getCurrentPlayer()
{
	assert(m_players[m_currentPlayerTurn].get());
	return m_players[m_currentPlayerTurn];
}

void Battle::onResetBattle()
{
	m_currentBattlePhase = BattlePhase::Deployment;
	m_currentPlayerTurn = 0;
	m_currentLightIntensity = eLightIntensity::eMaximum;
	m_lightIntensityTimer.reset();
	m_players.clear();
}

void Battle::incrementPlayerTurn()
{
	int wind = rand() % eDirection::Max;
	m_map.setWindDirection((eDirection)wind);
	
	++m_currentPlayerTurn;
	if (m_currentPlayerTurn == static_cast<int>(m_players.size()))
	{
		m_currentPlayerTurn = 0;
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
	return m_players[m_currentPlayerTurn]->m_factionName;
}

ePlayerType Battle::getCurrentPlayerType() const
{
	assert(m_players[m_currentPlayerTurn].get());
	return m_players[m_currentPlayerTurn]->m_playerType;
}

const Player & Battle::getPlayer(FactionName factionName) const
{
	auto cIter = std::find_if(m_players.cbegin(), m_players.cend(), [factionName](auto& player) { return player->m_factionName == factionName; });
	assert(cIter != m_players.cend());
	return *cIter->get();
}

void Battle::onYellowShipDestroyed()
{
	m_winningFactionHandler.onYellowShipDestroyed(m_players);
}

void Battle::onBlueShipDestroyed()
{
	m_winningFactionHandler.onBlueShipDestroyed(m_players);
}

void Battle::onGreenShipDestroyed()
{
	m_winningFactionHandler.onGreenShipDestroyed(m_players);
}

void Battle::onRedShipDestroyed()
{
	m_winningFactionHandler.onRedShipDestroyed(m_players);
}

void Battle::onEndMovementPhaseEarly()
{
	bool actionBeingPerformed = false;
	for (auto& entity : m_players[m_currentPlayerTurn]->m_ships)
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

void Battle::WinningFactionHandler::update(float deltaTime)
{
	m_winTimer.update(deltaTime);
	if (m_winTimer.isExpired())
	{
		switch (m_winningFaction)
		{
		case FactionName::eYellow :
			GameEventMessenger::getInstance().broadcast(GameEvent::eYellowWin);
			break;
		case FactionName::eBlue :
			GameEventMessenger::getInstance().broadcast(GameEvent::eBlueWin);
			break;
		case FactionName::eRed :
			GameEventMessenger::getInstance().broadcast(GameEvent::eRedWin);
			break;
		case FactionName::eGreen :
			GameEventMessenger::getInstance().broadcast(GameEvent::eGreenWin);
			break;
		}

		m_gameOver = true;
	}
}

void Battle::WinningFactionHandler::onYellowShipDestroyed(std::vector<std::unique_ptr<Player>>& players)
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

void Battle::WinningFactionHandler::onBlueShipDestroyed(std::vector<std::unique_ptr<Player>>& players)
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

void Battle::WinningFactionHandler::onGreenShipDestroyed(std::vector<std::unique_ptr<Player>>& players)
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

void Battle::WinningFactionHandler::onRedShipDestroyed(std::vector<std::unique_ptr<Player>>& players)
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

void Battle::WinningFactionHandler::checkGameStatus(const std::vector<std::unique_ptr<Player>>& players)
{
	//Check to see if all players have been eliminated
	int playersEliminated = 0;
	for (const auto& player : players)
	{
		if (player->m_eliminated)
		{
			++playersEliminated;
		}
	}
	//Last player standing - Player wins
	if (playersEliminated == static_cast<int>(players.size()) - 1)
	{
		auto player = std::find_if(players.cbegin(), players.cend(), [](const auto& player) { return player->m_eliminated == false; });
		assert(player != players.cend());
		FactionName winningFaction = player->get()->m_factionName;
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