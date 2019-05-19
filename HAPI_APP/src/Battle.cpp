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
	if (!m_AITurn)
	{
		m_timeUntilAITurn.reset();
		m_timeBetweenAIUnits.reset();
		return;
	}
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
		for (auto& it : m_players[m_currentPlayerTurn].m_entities)
		{
			if (!it->m_battleProperties.isDead() &&
				!it->m_battleProperties.isDestinationSet())
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
	if (!m_AITurn)
	{
		m_timeUntilAITurn.reset();
		m_timeBetweenAIUnits.reset();
		return;
	}
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
		for (auto& it : m_players[m_currentPlayerTurn].m_entities)
		{
			if (!it->m_battleProperties.isDead() &&
				!it->m_battleProperties.isWeaponFired())
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

Battle::Battle()
	: m_players(),
	m_currentPlayerTurn(0),
	m_map(),
	m_currentPhase(BattlePhase::Deployment),
	m_battleUI(*this),
	m_explosionParticles(),
	m_fireParticles(),
	m_timeUntilAITurn(1.5f, false),
	m_timeBetweenAIUnits(0.3f, false),
	m_AITurn(false),
	m_lightIntensity()
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

void Battle::start(const std::string & newMapName, const std::vector<Player>& newPlayers)
{
	assert(!newPlayers.empty());
	assert(m_players.empty());
	m_map.loadmap(newMapName);
	m_battleUI.loadGUI(m_map.getDimensions());
	//TODO: Hack to correct sprite sizes
	for (auto& player : newPlayers)
	{
		for (auto& entity : player.m_entities)
		{
			entity.m_sprite->GetTransformComp().SetOriginToCentreOfFrame();
			entity.m_sprite->GetTransformComp().SetScaling({ 1, 1 });
		}
	}
	//Create all players
	for (auto& player : newPlayers)
	{
		auto spawnPosition = m_map.getSpawnPosition();
		m_players.emplace_back(player.m_factionName, spawnPosition, player.m_type);
	}
	//Cycle through human players
	m_battleUI.deployHumanPlayers(newPlayers, m_map, *this);
	//Cycle through AI deployments
	for (auto& player : newPlayers)
	{
		if (player.m_type == ePlayerType::eAI)
		{
			AI::handleDeploymentPhase(*this, m_map, this->getPlayer(player.m_factionName), player);
		}
	}
	m_battleUI.loadGUI(m_map.getDimensions());
	if (m_battleUI.isHumanDeploymentCompleted())
		nextTurn();
}

void Battle::render() const
{
	m_map.drawMap(m_lightIntensity.m_lightIntensity);

	for (const auto& player : m_players)
	{
		for (const auto& entity : player.m_entities)
		{
			entity->m_battleProperties.renderPath(m_map);
		}
	}
	m_battleUI.renderUI();
	for (const auto& player : m_players)
	{
		for (const auto& entity : player.m_entities)
		{
			entity->m_battleProperties.render(entity->m_entityProperties.m_sprite, m_map);
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
	if (m_battleManager.isGameOver())
	{
		return;
	}

	m_battleUI.setCurrentFaction(getCurrentFaction());
	m_battleUI.update(deltaTime);
	m_map.setDrawOffset(m_battleUI.getCameraPositionOffset());

	if (!m_battleUI.isPaused())
	{
		m_lightIntensity.update(deltaTime);
		for (auto& explosionParticle : m_explosionParticles)
		{
			explosionParticle.run(deltaTime, m_map);
		}
		for (auto& fireParticle : m_fireParticles)
		{
			fireParticle.run(deltaTime, m_map);
		}

		if (m_currentPhase == BattlePhase::Movement)
		{
			updateMovementPhase(deltaTime);
			handleAIMovementPhaseTimer(deltaTime);
		}
		else if (m_currentPhase == BattlePhase::Attack)
		{
			updateAttackPhase();
			handleAIAttackPhaseTimer(deltaTime);
		}
	}

	m_battleManager.update(deltaTime);
}

void Battle::moveEntityToPosition(BattleEntity& entity, const Tile& destination)
{
	assert(m_currentPhase == BattlePhase::Movement);
	entity.m_battleProperties.moveEntity(m_map, destination);
}

void Battle::moveEntityToPosition(BattleEntity& entity, const Tile& destination, eDirection endDirection)
{
	assert(m_currentPhase == BattlePhase::Movement);
	entity.m_battleProperties.moveEntity(m_map, destination, endDirection);
}

bool Battle::fireEntityWeaponAtPosition(const Tile& tileOnPlayer, const Tile& tileOnAttackPosition, const std::vector<const Tile*>& targetArea)
{
	assert(m_currentPhase == BattlePhase::Attack);
	assert(tileOnPlayer.m_entityOnTile);

	if (!tileOnPlayer.m_entityOnTile)
	{
		return false;
	}
	assert(!tileOnPlayer.m_entityOnTile->m_battleProperties.isWeaponFired());

	//Disallow attacking same team
	if (tileOnAttackPosition.m_entityOnTile && tileOnAttackPosition.m_entityOnTile->m_factionName != getCurrentFaction()
			&& !tileOnAttackPosition.m_entityOnTile->m_battleProperties.isDead())
	{
		//Find entity 
		auto tileCoordinate = tileOnAttackPosition.m_entityOnTile->m_battleProperties.getCurrentPosition();
		auto cIter = std::find_if(targetArea.cbegin(), targetArea.cend(), [tileCoordinate](const auto& tile) { if(tile) return tileCoordinate == tile->m_tileCoordinate; });
		//Enemy within range of weapon
		if (cIter != targetArea.cend())
		{
			if (tileOnPlayer.m_entityOnTile->m_entityProperties.m_weaponType == eFlamethrower)
			{
				playFireAnimation(*tileOnPlayer.m_entityOnTile, targetArea[0]->m_tileCoordinate);
			}
			else
			{
				playExplosionAnimation(*tileOnAttackPosition.m_entityOnTile);
			}

			tileOnPlayer.m_entityOnTile->m_battleProperties.fireWeapon();
			auto& enemy = tileOnAttackPosition.m_entityOnTile;
			enemy->m_battleProperties.takeDamage(enemy->m_entityProperties, tileOnPlayer.m_entityOnTile->m_entityProperties.m_damage, enemy->m_factionName);
			
			return true;
		}
	}

	return false;
}

void Battle::insertEntity(std::pair<int, int> startingPosition, eDirection startingDirection, const EntityProperties& entityProperties, FactionName factionName)
{
	assert(m_currentPhase == BattlePhase::Deployment);

	auto& player = getPlayer(factionName);
	player.m_entities.push_back(std::make_unique<BattleEntity>(startingPosition, entityProperties, m_map, factionName, startingDirection));
}

void Battle::nextTurn()
{
	FactionName currentPlayer;
	bool lastPlayer = false;

	switch (m_currentPhase)
	{
	case BattlePhase::Deployment :
		lastPlayer = (m_battleUI.isHumanDeploymentCompleted());
		incrementPlayerTurn();
		if (lastPlayer)
		{
			m_currentPhase = BattlePhase::Movement;

			//for (auto& entity : m_players[m_currentPlayerTurn].m_entities)
			//{
			//	entity->m_battleProperties.enableAction();
			//}
			
			m_currentPlayerTurn = 0;
			if (m_players[m_currentPlayerTurn].m_playerType == ePlayerType::eAI)
			{
				m_timeUntilAITurn.setActive(true);
				GameEventMessenger::getInstance().broadcast(GameEvent::eEnteredAITurn);
				m_AITurn = true;
			}
		}
		GameEventMessenger::getInstance().broadcast(GameEvent::eNewTurn);
		currentPlayer = m_players[m_currentPlayerTurn].m_factionName;
		for (auto& entity : m_players[m_currentPlayerTurn].m_entities)
		{
			entity->m_battleProperties.enableAction();
		}
		break;
	case BattlePhase::Movement :
		m_currentPhase = BattlePhase::Attack;
		//resetAITimers();
		GameEventMessenger::getInstance().broadcast(GameEvent::eNewTurn);
		GameEventMessenger::getInstance().broadcast(GameEvent::eEnteringAttackPhase);

		for (auto& entity : m_players[m_currentPlayerTurn].m_entities)
		{
			entity->m_battleProperties.enableAction();
		}

		if (!m_players[m_currentPlayerTurn].m_eliminated && m_players[m_currentPlayerTurn].m_playerType == ePlayerType::eAI)
		{
			m_timeUntilAITurn.setActive(true);
			GameEventMessenger::getInstance().broadcast(GameEvent::eEnteredAITurn);
			m_AITurn = true;
		}
		break;
	case BattlePhase::Attack :
		m_currentPhase = BattlePhase::Movement;
		//resetAITimers();
		GameEventMessenger::getInstance().broadcast(GameEvent::eNewTurn);
		GameEventMessenger::getInstance().broadcast(GameEvent::eEnteringMovementPhase);

		for (auto& entity : m_players[m_currentPlayerTurn].m_entities)
		{
			entity->m_battleProperties.disableAction();
		}

		updateWindDirection();
		incrementPlayerTurn();
		for (auto& entity : m_players[m_currentPlayerTurn].m_entities)
		{
			entity->m_battleProperties.enableAction();
		}

		if (!m_players[m_currentPlayerTurn].m_eliminated && m_players[m_currentPlayerTurn].m_playerType == ePlayerType::eAI)
		{
			m_timeUntilAITurn.setActive(true);
			GameEventMessenger::getInstance().broadcast(GameEvent::eEnteredAITurn);
			m_AITurn = true;
		}
		else if (m_players[m_currentPlayerTurn].m_playerType == ePlayerType::eHuman)
		{
			GameEventMessenger::getInstance().broadcast(GameEvent::eLeftAITurn);
			m_AITurn = false;
		}
		break;
	}
}

std::vector<FactionName> Battle::getAllFactions() const
{
	std::vector<FactionName> lol;
	for (auto& player : m_players)
	{
		lol.emplace_back(player.m_factionName);
	}

	assert(!lol.empty());
	return lol;
}

void Battle::playFireAnimation(BattleEntity& entity, std::pair<int, int> position)
{
	for (auto& it : m_fireParticles)
	{
		if (!it.m_isEmitting)
		{
			it.orient(entity.m_battleProperties.getCurrentDirection());
			it.setPosition(position);
			it.m_isEmitting = true;
			return;
		}
	}
}

void Battle::playExplosionAnimation(BattleEntity& entity)
{
	for (auto& it : m_explosionParticles)
	{
		if (!it.m_isEmitting)
		{
			it.setPosition(entity.m_battleProperties.getCurrentPosition());
			it.m_isEmitting = true;
			return;
		}
	}
}

void Battle::updateMovementPhase(float deltaTime)
{
	for (auto& entity : m_players[m_currentPlayerTurn].m_entities)
	{
		if (entity->m_battleProperties.isDead())
			continue;
		entity->m_battleProperties.update(deltaTime, m_map, entity->m_entityProperties);
	}
	for (auto& entity : m_players[m_currentPlayerTurn].m_entities)
	{
		if (entity->m_battleProperties.isDead())
			continue;
		if (!entity->m_battleProperties.isDestinationSet())
		{
			return;
		}
		if (entity->m_battleProperties.isMovingToDestination())
		{
			return;
		}
	}

	nextTurn();
}

void Battle::updateAttackPhase()
{
	if (allEntitiesAttacked(m_players[m_currentPlayerTurn].m_entities))
	{	
		nextTurn();
	}
}

bool Battle::allEntitiesAttacked(std::vector<std::shared_ptr<BattleEntity>>& playerEntities) const
{
	bool allEntitiesAttacked = true;
	for (const auto& entity : playerEntities)
	{
		if (!entity->m_battleProperties.isDead() && !entity->m_battleProperties.isWeaponFired())
		{
			allEntitiesAttacked = false;
		}
	}

	return allEntitiesAttacked;
}

BattlePlayer& Battle::getPlayer(FactionName factionName)
{
	auto cIter = std::find_if(m_players.begin(), m_players.end(), [factionName](const auto& player) { return factionName == player.m_factionName; });
	assert(cIter != m_players.end());
	return *cIter;
}

void Battle::onResetBattle()
{
	m_currentPhase = BattlePhase::Deployment;
	m_currentPlayerTurn = 0;
	m_lightIntensity.m_lightIntensity = eLightIntensity::eMaximum;
	m_lightIntensity.m_timer.reset();
	m_players.clear();
}

void Battle::incrementPlayerTurn()
{
	++m_currentPlayerTurn;
	int wind = rand() % eDirection::Max;
	m_map.setWindDirection((eDirection)wind);
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
	return m_currentPhase;
}

FactionName Battle::getCurrentFaction() const
{
	return m_players[m_currentPlayerTurn].m_factionName;
}

const BattlePlayer & Battle::getPlayer(FactionName factionName) const
{
	auto cIter = std::find_if(m_players.cbegin(), m_players.cend(), [factionName](const auto& player) { return player.m_factionName == factionName; });
	assert(cIter != m_players.cend());
	return *cIter;
}

bool Battle::isAIPlaying() const
{
	return m_AITurn;
}

void Battle::onYellowShipDestroyed()
{
	m_battleManager.onYellowShipDestroyed(m_players);
}

void Battle::onBlueShipDestroyed()
{
	m_battleManager.onBlueShipDestroyed(m_players);
}

void Battle::onGreenShipDestroyed()
{
	m_battleManager.onGreenShipDestroyed(m_players);
}

void Battle::onRedShipDestroyed()
{
	m_battleManager.onRedShipDestroyed(m_players);
}

void Battle::onEndMovementPhaseEarly()
{
	bool actionBeingPerformed = false;
	for (auto& entity : m_players[m_currentPlayerTurn].m_entities)
	{
		if (entity->m_battleProperties.isMovingToDestination())
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

Battle::BattleManager::BattleManager()
	: m_yellowShipsDestroyed(0),
	m_blueShipsDestroyed(0),
	m_greenShipsDestroyed(0),
	m_redShipsDestroyed(0),
	m_winTimer(2.0f, false),
	m_gameOver(false)
{
	GameEventMessenger::getInstance().subscribe(std::bind(&BattleManager::onReset, this), "BattleManager", GameEvent::eResetBattle);
}

Battle::BattleManager::~BattleManager()
{
	GameEventMessenger::getInstance().unsubscribe("BattleManager", GameEvent::eResetBattle);
}

bool Battle::BattleManager::isGameOver() const
{
	return m_gameOver;
}

void Battle::BattleManager::update(float deltaTime)
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

void Battle::BattleManager::onYellowShipDestroyed(std::vector<BattlePlayer>& players)
{
	++m_yellowShipsDestroyed;
	auto player = std::find_if(players.begin(), players.end(), [](const auto& player) { return player.m_factionName == FactionName::eYellow; });
	assert(player != players.end());
	if (m_yellowShipsDestroyed == static_cast<int>(player->m_entities.size()))
	{
		player->m_eliminated = true;
		checkGameStatus(players);
	}
}

void Battle::BattleManager::onBlueShipDestroyed(std::vector<BattlePlayer>& players)
{
	++m_blueShipsDestroyed;
	auto player = std::find_if(players.begin(), players.end(), [](const auto& player) { return player.m_factionName == FactionName::eBlue; });
	assert(player != players.end());
	if (m_blueShipsDestroyed == static_cast<int>(player->m_entities.size()))
	{
		player->m_eliminated = true;
		checkGameStatus(players);
	}
}

void Battle::BattleManager::onGreenShipDestroyed(std::vector<BattlePlayer>& players)
{
	++m_greenShipsDestroyed;
	auto player = std::find_if(players.begin(), players.end(), [](const auto& player) { return player.m_factionName == FactionName::eGreen; });
	assert(player != players.end());
	if (m_greenShipsDestroyed == static_cast<int>(player->m_entities.size()))
	{
		player->m_eliminated = true;
		checkGameStatus(players);
	}
}

void Battle::BattleManager::onRedShipDestroyed(std::vector<BattlePlayer>& players)
{
	++m_redShipsDestroyed;
	auto player = std::find_if(players.begin(), players.end(), [](const auto& player) { return player.m_factionName == FactionName::eRed; });
	assert(player != players.end());
	if (m_redShipsDestroyed == static_cast<int>(player->m_entities.size()))
	{
		player->m_eliminated = true;
		checkGameStatus(players);
	}
}

void Battle::BattleManager::onReset()
{
	m_yellowShipsDestroyed = 0;
	m_redShipsDestroyed = 0;
	m_blueShipsDestroyed = 0;
	m_greenShipsDestroyed = 0;
	m_winTimer.reset();
	m_winTimer.setActive(false);
	m_gameOver = false;
}

void Battle::BattleManager::checkGameStatus(const std::vector<BattlePlayer>& players)
{
	//Check to see if all players have been eliminated
	int playersEliminated = 0;
	for (const auto& player : players)
	{
		if (player.m_eliminated)
		{
			++playersEliminated;
		}
	}
	//Last player standing - Player wins
	if (playersEliminated == static_cast<int>(players.size()) - 1)
	{
		auto player = std::find_if(players.cbegin(), players.cend(), [](const auto& player) { return player.m_eliminated == false; });
		assert(player != players.cend());
		FactionName winningFaction = player->m_factionName;
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

Battle::LightIntensity::LightIntensity()
	: m_timer(30.0f),
	m_lightIntensity(eLightIntensity::eMaximum)
{}

void Battle::LightIntensity::update(float deltaTime)
{
	m_timer.update(deltaTime);
	if (m_timer.isExpired())
	{
		if (m_lightIntensity == eLightIntensity::eMaximum)
		{
			m_lightIntensity = eLightIntensity::eMinimum;
			m_timer.setNewExpirationTime(15.f);
			m_timer.reset();
		}
		else
		{
			m_lightIntensity = eLightIntensity::eMaximum;
			m_timer.setNewExpirationTime(30.f);
			m_timer.reset();
		}
	}
}
