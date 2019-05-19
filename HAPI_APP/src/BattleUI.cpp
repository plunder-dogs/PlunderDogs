#include "BattleUI.h"
#include "Battle.h"
#include "Pathfinding.h"
#include "OverWorld.h"
#include "Textures.h"
#include "MouseSelection.h"
#include "GameEventMessenger.h"
#include <assert.h>
#include "AI.h"
using namespace HAPISPACE;
constexpr float DRAW_ENTITY_OFFSET_X{ 16 };
constexpr float DRAW_ENTITY_OFFSET_Y{ 32 };
constexpr int SHIP_PLACEMENT_SPAWN_RANGE{ 3 };

//
//InvalidPositionSprite
//
BattleUI::InvalidPosition::InvalidPosition()
	: m_sprite(std::make_unique<Sprite>(Textures::m_thing)),
	m_activate(false)
{
	m_sprite->GetTransformComp().SetOriginToCentreOfFrame();
}

void BattleUI::InvalidPosition::render(const Map& map) const
{
	if (m_activate)
	{
		const std::pair<int, int> tileTransform = map.getTileScreenPos(m_position);

		m_sprite->GetTransformComp().SetPosition({
		static_cast<float>(tileTransform.first + DRAW_ENTITY_OFFSET_X * map.getDrawScale()),
		static_cast<float>(tileTransform.second + DRAW_ENTITY_OFFSET_Y * map.getDrawScale()) });

		m_sprite->Render(SCREEN_SURFACE);
	}
}

void BattleUI::InvalidPosition::setPosition(std::pair<int, int> newPosition, const Map& map)
{
	m_sprite->GetTransformComp().SetPosition({
		(float)newPosition.first + DRAW_ENTITY_OFFSET_X * map.getDrawScale(),
		(float)newPosition.second + DRAW_ENTITY_OFFSET_Y * map.getDrawScale()});

	m_position = newPosition;
}

void BattleUI::InvalidPosition::onReset()
{
	m_activate = false;
	m_position = std::pair<int, int>(0, 0);
}

//
//BattleUI
//
BattleUI::BattleUI(Battle & battle)
	: m_battle(battle),
	m_gui(),
	m_selectedTile(),
	m_invalidPosition(),
	m_leftMouseDownPosition({ 0, 0 }),
	m_isMovingEntity(false),
	m_mouseDownTile(nullptr),
	m_arrowActive(false),
	m_arrowSprite(HAPI_Sprites.MakeSprite(Textures::m_CompassPointer)),
	m_lastMouseData({0,0})
{
	m_arrowSprite->GetTransformComp().SetScaling({ 0.5, 0.5 });
	m_arrowSprite->GetTransformComp().SetOriginToCentreOfFrame();
	GameEventMessenger::getInstance().subscribe(std::bind(&BattleUI::onResetBattle, this), "BattleUI", GameEvent::eResetBattle);
	GameEventMessenger::getInstance().subscribe(std::bind(&BattleUI::onNewTurn, this), "BattleUI", GameEvent::eNewTurn);
}

BattleUI::~BattleUI()
{
	GameEventMessenger::getInstance().unsubscribe("BattleUI", GameEvent::eResetBattle);
	GameEventMessenger::getInstance().unsubscribe("BattleUI", GameEvent::eNewTurn);
}

std::pair<int, int> BattleUI::getCameraPositionOffset() const
{
	return m_gui.getCameraPositionOffset();
}

int BattleUI::isHumanDeploymentCompleted() const
{
	return m_shipDeployment.empty();
}

void BattleUI::renderUI() const
{
	switch (m_battle.getCurrentPhase())
	{
	case BattlePhase::Deployment:
		assert(!m_shipDeployment.empty());
		m_shipDeployment.front()->render(m_invalidPosition, m_battle.getMap());
		renderArrow();
		break;

	case BattlePhase::Movement:
		m_selectedTile.render(m_battle.getMap());
		renderArrow();
		break;
	
	case BattlePhase::Attack:
		m_selectedTile.render(m_battle.getMap());
		break;
	}
}

void BattleUI::renderGUI() const
{
	m_invalidPosition.render(m_battle.getMap());
	m_gui.render(m_battle.getCurrentPhase());

	if (m_selectedTile.m_tile  != nullptr && m_selectedTile.m_tile->m_entityOnTile != nullptr)
	{
		m_gui.renderStats(m_selectedTile.m_tile->m_entityOnTile->m_entityProperties);
	}
}

void BattleUI::loadGUI(std::pair<int, int> mapDimensions)
{
	m_gui.setMaxCameraOffset(mapDimensions);
}

void BattleUI::drawTargetArea() const
{
	for (const auto& i : m_targetArea.m_targetAreaSprites)
	{
		if (i.activate)
		{
			const std::pair<int, int> tileTransform = m_battle.getMap().getTileScreenPos(i.position);

			i.sprite->GetTransformComp().SetPosition({
			static_cast<float>(tileTransform.first + DRAW_ENTITY_OFFSET_X * m_battle.getMap().getDrawScale()),
			static_cast<float>(tileTransform.second + DRAW_ENTITY_OFFSET_Y * m_battle.getMap().getDrawScale()) });

			i.sprite->Render(SCREEN_SURFACE);
		}
	}
}

void BattleUI::update(float deltaTime)
{
	m_gui.update(m_battle.getMap().getWindDirection());// added update for gui to receive wind direction so compass direction updates
}

void BattleUI::FactionUpdateGUI(FactionName faction)
{
	m_gui.updateFactionToken(faction);
}

void BattleUI::deployHumanPlayers(const std::vector<Player>& newPlayers, Map& map, const Battle& battle)
{
	assert(m_battle.getCurrentPhase() == BattlePhase::Deployment);
	assert(m_shipDeployment.empty());

	bool positionToSnapToChosen = false;
	std::pair<int, int> positionToSnapTo;

	for (const auto& player : newPlayers)
	{
		if(player.m_type == ePlayerType::eHuman)
		{
			if (!positionToSnapToChosen)
			{
				positionToSnapToChosen = true;
				positionToSnapTo = battle.getPlayer(player.m_factionName).m_spawnPosition;
			}

			m_shipDeployment.push_back(std::make_unique<DeploymentPhase>(player.m_selectedEntities,
				battle.getPlayer(player.m_factionName).m_spawnPosition, SHIP_PLACEMENT_SPAWN_RANGE, m_battle.getMap(), player.m_factionName));
		}
	}

	//Snap the Camera to the first position of play
	m_gui.snapCameraToPosition(positionToSnapTo);
}

void BattleUI::OnMouseEvent(EMouseEvent mouseEvent, const HAPI_TMouseData & mouseData)
{
	m_lastMouseData = { mouseData.x , mouseData.y };
	if (OverWorldGUI::CURRENT_WINDOW != OverWorldWindow::eBattle)
	{
		return;
	}
	if (mouseEvent == EMouseEvent::eLeftButtonDown)
	{
		m_gui.OnMouseLeftClick(mouseData, m_battle.getCurrentPhase());

		switch (m_battle.getCurrentPhase())
		{
		case BattlePhase::Deployment :
		{
			m_isMovingEntity = true;
			m_leftMouseDownPosition = { mouseData.x, mouseData.y };
			break;
		}
		case BattlePhase::Movement :
		{
			m_leftMouseDownPosition = { mouseData.x, mouseData.y };
			onLeftClickMovementPhase();
			break;
		}
		case BattlePhase::Attack :
		{
			onLeftClickAttackPhase();
			break;
		}
		}
	}
	else if (mouseEvent == EMouseEvent::eRightButtonDown)
	{
		switch (m_battle.getCurrentPhase())
		{
		case BattlePhase::Movement :
		{
			onRightClickMovementPhase();
			break;
		}
		case BattlePhase::Attack :
		{
			onRightClickAttackPhase();
			break;
		}
		}
	}
	else if (mouseEvent == EMouseEvent::eLeftButtonUp)
	{
		if (m_isMovingEntity)
		{
			std::pair<double, eDirection> mouseMoveDirection{ MouseSelection::calculateDirection(m_leftMouseDownPosition,HAPI_Wrapper::getMouseLocation()) };
			switch (m_battle.getCurrentPhase())
			{
			case BattlePhase::Deployment :
			{
				assert(!m_shipDeployment.empty());
				if (!m_selectedTile.m_tile)
					break;
				if (mouseMoveDirection.first > 20)
				{
					m_shipDeployment.front()->onLeftClick(m_invalidPosition, mouseMoveDirection.second, m_selectedTile.m_tile, m_battle);
					if (m_shipDeployment.front()->isCompleted())
					{
						m_shipDeployment.pop_front();
						if (!m_shipDeployment.empty())
						{
							m_selectedTile.m_tile = nullptr;
							m_invalidPosition.m_activate = true;
							m_gui.snapCameraToPosition(m_shipDeployment.front()->getSpawnPosition());
						}
						else
						{
							m_selectedTile.m_tile = nullptr;
							m_invalidPosition.m_activate = true;
							m_battle.nextTurn();
						}
						
					}
					//m_battle.moveEntityToPosition(*m_selectedTile.m_tile->m_entityOnTile, *m_battle.getMap().getTile(m_leftMouseDownPosition), mouseMoveDirection.second);
				}
				else
				{
					//This function call is to be used if the movement of the mouse during the move command is small enough to be considered unintended,
					//in this case the ship should not rotate after reaching the destination.
					m_shipDeployment.front()->onLeftClick(m_invalidPosition, eNorth, m_selectedTile.m_tile, m_battle);
					if (m_shipDeployment.front()->isCompleted())
					{
						m_shipDeployment.pop_front();
						if (!m_shipDeployment.empty())
						{
							m_selectedTile.m_tile = nullptr;
							m_gui.snapCameraToPosition(m_shipDeployment.front()->getSpawnPosition());
						}
						else
						{
							m_selectedTile.m_tile = nullptr;
							m_battle.nextTurn();
						}
					}
				}
				m_arrowActive = false;
				break;
			}
			case BattlePhase::Movement :
			{
				if (!m_mouseDownTile || !m_selectedTile.m_tile)
				{
					break;
				}
				if (mouseMoveDirection.first > 20)
				{
					m_battle.moveEntityToPosition(*m_selectedTile.m_tile->m_entityOnTile, *m_mouseDownTile, mouseMoveDirection.second);
				}
				else
				{
					m_battle.moveEntityToPosition(*m_selectedTile.m_tile->m_entityOnTile, *m_mouseDownTile);
				}
				m_arrowActive = false;
				break;
			}
			}
			//Resetting the variables used as triggers
			m_mouseDownTile = nullptr;
			//TODO: Drop info box
			m_selectedTile.m_tile = nullptr;
			m_isMovingEntity = false;
		}
	}
}

void BattleUI::OnMouseMove(const HAPI_TMouseData & mouseData)
{
	m_lastMouseData = { mouseData.x , mouseData.y };
	if (OverWorldGUI::CURRENT_WINDOW != OverWorldWindow::eBattle)
	{
		return;
	}

	m_gui.OnMouseMove(mouseData, m_battle.getCurrentPhase());

	switch (m_battle.getCurrentPhase())
	{
	case BattlePhase::Deployment:
	{
		assert(!m_shipDeployment.empty());
		if (!m_isMovingEntity)
		{
			m_selectedTile.m_tile = m_shipDeployment.front()->getTileOnMouse(
				m_invalidPosition, m_selectedTile.m_tile, m_battle.getMap());
			//TODO: Raise info box
		}
		break;
	}
	case BattlePhase::Movement:
	{
		onMouseMoveMovementPhase();
		break;
	}
	case BattlePhase::Attack :
	{
		onMouseMoveAttackPhase();
		break;
	}
	}
}

void BattleUI::setCurrentFaction(FactionName faction)
{
	FactionUpdateGUI(faction);
}

bool BattleUI::isPaused()
{
	return m_gui.isPaused();
}

void BattleUI::clearTargetArea()
{
	m_targetArea.clearTargetArea();
}

void BattleUI::clearSelectedTile()
{
	m_arrowActive = false;
	if (m_selectedTile.m_tile && m_selectedTile.m_tile->m_entityOnTile)
		m_selectedTile.m_tile->m_entityOnTile->m_battleProperties.clearMovementPath();
	m_selectedTile.m_tile = nullptr;
}

void BattleUI::renderArrow() const
{
	if (!m_arrowActive || !m_mouseDownTile)
		return;
	auto directionData = MouseSelection::calculateDirection(m_leftMouseDownPosition, m_lastMouseData);
	if (directionData.first < 20)
		return;
	int windDirection = static_cast<int>(directionData.second);
	std::pair<int, int> pos = m_battle.getMap().getTileScreenPos(m_mouseDownTile->m_tileCoordinate);
	float scale = m_battle.getMap().getDrawScale();
	m_arrowSprite->GetTransformComp().SetPosition({ static_cast<float>(pos.first + (16 * scale)), static_cast<float>(pos.second + (32 * scale)) });
	m_arrowSprite->GetTransformComp().SetRotation(((windDirection * 60) % 360) * M_PI / 180.0f);
	m_arrowSprite->Render(SCREEN_SURFACE);
}

void BattleUI::onMouseMoveMovementPhase()
{
	assert(m_battle.getCurrentPhase() == BattlePhase::Movement);

	if (m_battle.isAIPlaying())
	{
		return;
	}

	//Current tile selected does not match the current player in play
	if (m_selectedTile.m_tile && m_selectedTile.m_tile->m_entityOnTile 
		&& m_selectedTile.m_tile->m_entityOnTile->m_factionName != m_battle.getCurrentFaction())
	{
		return;
	}

	//!isMovingToDestination && !reachedDestination
	if (m_selectedTile.m_tile && m_selectedTile.m_tile->m_entityOnTile &&  
		!m_selectedTile.m_tile->m_entityOnTile->m_battleProperties.isMovingToDestination() &&
		!m_selectedTile.m_tile->m_entityOnTile->m_battleProperties.isDestinationSet())
	{
		const Tile* tile = m_battle.getMap().getTile(m_battle.getMap().getMouseClickCoord(HAPI_Wrapper::getMouseLocation()));
		if (!tile)
		{
			return;
		}

		if (m_selectedTile.m_tile->m_tileCoordinate != tile->m_tileCoordinate && !m_mouseDownTile)
		{
			if (tile->m_type != eTileType::eSea && tile->m_type != eTileType::eOcean)
			{
				m_invalidPosition.setPosition(tile->m_tileCoordinate, m_battle.getMap());
				m_invalidPosition.m_activate = true;
				m_selectedTile.m_tile->m_entityOnTile->m_battleProperties.clearMovementPath();
			}
			else
			{
				m_selectedTile.m_tile->m_entityOnTile->m_battleProperties.generateMovementGraph(m_battle.getMap(), *m_selectedTile.m_tile, *tile);
				m_invalidPosition.m_activate = false;
			}
		}
	}
}

void BattleUI::onLeftClickMovementPhase()
{
	assert(m_battle.getCurrentPhase() == BattlePhase::Movement);
	
	const Tile* tileOnMouse = m_battle.getMap().getTile(m_battle.getMap().getMouseClickCoord(HAPI_Wrapper::getMouseLocation()));
	if (!tileOnMouse)
	{
		return;
	}

	if (!m_selectedTile.m_tile && tileOnMouse->m_entityOnTile)
	{
		m_selectedTile.m_tile = tileOnMouse;
		if (!m_battle.isAIPlaying())
		{
			m_selectedTile.m_tile->m_entityOnTile->m_battleProperties.clearMovementPath();
			//TODO: Trigger movement area showing
		}
		return;
	}

	//AI is in play
	if (m_battle.isAIPlaying())
	{
		return;
	}

	if (!m_selectedTile.m_tile && tileOnMouse->m_entityOnTile &&
		tileOnMouse->m_entityOnTile->m_factionName != m_battle.getCurrentFaction())
	{
		m_selectedTile.m_tile = tileOnMouse;
		return;
	}

	//Invalid Location - Collidable tile
	if (tileOnMouse->m_type != eTileType::eSea && tileOnMouse->m_type != eTileType::eOcean)
	{
		if (m_selectedTile.m_tile && m_selectedTile.m_tile->m_entityOnTile)
		{
			m_selectedTile.m_tile->m_entityOnTile->m_battleProperties.clearMovementPath();
		}
		return;
	}

	//Do not select killed entity
	if (tileOnMouse->m_entityOnTile && tileOnMouse->m_entityOnTile->m_battleProperties.isDead())
	{
		//TODO: Drop info box
		m_selectedTile.m_tile = nullptr;
		return;
	}

	//Clicking to where entity is moving to
	if (tileOnMouse->m_entityOnTile && tileOnMouse->m_entityOnTile->m_battleProperties.isDestinationSet())
	{
		//TODO: Drop info box
		m_selectedTile.m_tile = nullptr;
		return;
	}

	if (m_selectedTile.m_tile)
	{
		//Cancel movement if clicked on same entity
		if (m_selectedTile.m_tile->m_tileCoordinate == tileOnMouse->m_tileCoordinate)
		{
			m_selectedTile.m_tile->m_entityOnTile->m_battleProperties.clearMovementPath();
			//TODO: Drop info box
			m_selectedTile.m_tile = nullptr;
		}

		//TODO: TAKE A LONG HARD LOOK AT THIS STATEMENT
		//Disallow movement to tile occupied by other player
		else if (tileOnMouse->m_entityOnTile && tileOnMouse->m_entityOnTile->m_factionName != m_battle.getCurrentFaction())
		{
			m_selectedTile.m_tile->m_entityOnTile->m_battleProperties.clearMovementPath();
			m_selectedTile.m_tile = tileOnMouse;
		}
		else if (tileOnMouse->m_entityOnTile && tileOnMouse->m_entityOnTile->m_factionName == m_battle.getCurrentFaction())
		{
			m_selectedTile.m_tile->m_entityOnTile->m_battleProperties.clearMovementPath();
			m_selectedTile.m_tile = tileOnMouse;
		}

		//Store data so Entity can move to new location
		else if (m_selectedTile.m_tile->m_entityOnTile && (m_selectedTile.m_tile->m_tileCoordinate != tileOnMouse->m_tileCoordinate)
			&& m_selectedTile.m_tile->m_entityOnTile->m_factionName == m_battle.getCurrentFaction())
		{
			assert(m_selectedTile.m_tile->m_entityOnTile->m_factionName == m_battle.getCurrentFaction());
			m_mouseDownTile = tileOnMouse;
			m_isMovingEntity = true;
			auto test = m_selectedTile.m_tile->m_entityOnTile->m_battleProperties.getEndOfPath();
			if (m_selectedTile.m_tile->m_entityOnTile->m_battleProperties.getEndOfPath() == tileOnMouse->m_tileCoordinate)
				m_arrowActive = true;
			//m_battle.moveEntityToPosition(*m_selectedTile.m_tile->m_entityOnTile, *tileOnMouse);
			//m_selectedTile.m_tile = nullptr;
		}
	}
	else
	{
		if (tileOnMouse->m_entityOnTile && tileOnMouse->m_entityOnTile->m_battleProperties.isDead())
		{
			//TODO: Drop info box
			m_selectedTile.m_tile = nullptr;
		}
		//Do not select tile that contains wrong players entity
		if (tileOnMouse->m_entityOnTile)
		{
			if (tileOnMouse->m_entityOnTile->m_factionName != m_battle.getCurrentFaction())
			{
				//TODO: Drop info box
				m_selectedTile.m_tile = nullptr;
			}
			else
			{
				//TODO: Raise info box
				m_selectedTile.m_tile = tileOnMouse;
			}
		}
	}
}

void BattleUI::onRightClickMovementPhase()
{
	assert(m_battle.getCurrentPhase() == BattlePhase::Movement);
	//Cancel selected Entity
	if (m_selectedTile.m_tile && m_selectedTile.m_tile->m_entityOnTile)
	{
		m_selectedTile.m_tile->m_entityOnTile->m_battleProperties.clearMovementPath();
		m_invalidPosition.m_activate = false;
	}
	//TODO: Drop info box
	m_selectedTile.m_tile = nullptr;
}

void BattleUI::onLeftClickAttackPhase()
{
	//assert(m_battle.getCurrentPhase() == BattlePhase::Attack);
	//const Tile* tileOnMouse = m_battle.getMap().getTile(m_battle.getMap().getMouseClickCoord(HAPI_Wrapper::getMouseLocation()));
	//if (!tileOnMouse || !tileOnMouse->m_entityOnTile)
	//{
	//	if (m_selectedTile.m_tile)
	//	{
	//		m_targetArea.clearTargetArea();
	//	}
	//	m_selectedTile.m_tile = nullptr;
	//	m_invalidPosition.m_activate = false;
	//	return;
	//}

	assert(m_battle.getCurrentPhase() == BattlePhase::Attack);
	const Tile* tileOnMouse = m_battle.getMap().getTile(m_battle.getMap().getMouseClickCoord(HAPI_Wrapper::getMouseLocation()));
	if (!tileOnMouse)
	{
		return;
	}

	//AI in play
	if (m_battle.isAIPlaying())
	{
		m_selectedTile.m_tile = tileOnMouse;
		return;
	}

	//Select new entity that is on same team
	if (m_selectedTile.m_tile && m_selectedTile.m_tile->m_entityOnTile && tileOnMouse->m_entityOnTile &&
		(tileOnMouse->m_entityOnTile->m_factionName == m_battle.getCurrentFaction()))
	{
		if (!tileOnMouse->m_entityOnTile->m_battleProperties.isWeaponFired())
		{
			m_targetArea.clearTargetArea();
			m_targetArea.generateTargetArea(m_battle.getMap(), *tileOnMouse);
			m_selectedTile.m_tile = tileOnMouse;
			return;
		}
	}

	if (!m_selectedTile.m_tile && tileOnMouse->m_entityOnTile && tileOnMouse->m_entityOnTile->m_factionName != m_battle.getCurrentFaction())
	{
		m_selectedTile.m_tile = tileOnMouse;
		return;
	}

	//Do not fire on destroyed ship
	if (tileOnMouse->m_entityOnTile && tileOnMouse->m_entityOnTile->m_battleProperties.isDead())
	{
		m_targetArea.clearTargetArea();
		m_invalidPosition.m_activate = false;
		//TODO: Drop info box
		m_selectedTile.m_tile = nullptr;
		return;
	}

	//Clicking on the same entity that has been previously selected
	if (m_selectedTile.m_tile && m_selectedTile.m_tile->m_tileCoordinate == tileOnMouse->m_tileCoordinate)
	{
		m_targetArea.clearTargetArea();
		m_invalidPosition.m_activate = false;
		//TODO: Drop info box
		m_selectedTile.m_tile = nullptr;
		return;
	}

	//Entity already selected Fire weapon at position
	//If the selectedTile has an entity on it and that entity hasn't fired
	if (m_selectedTile.m_tile && m_selectedTile.m_tile->m_entityOnTile && !m_selectedTile.m_tile->m_entityOnTile->m_battleProperties.isWeaponFired())
	{
		//If there is an entity on the tile you're clicking on and that entity's faction name differs from the one of the ship that's firing
		if ((tileOnMouse->m_entityOnTile != nullptr) && tileOnMouse->m_entityOnTile->m_factionName != m_selectedTile.m_tile->m_entityOnTile->m_factionName)
		{
			m_battle.fireEntityWeaponAtPosition(*m_selectedTile.m_tile, *tileOnMouse, m_targetArea.m_targetArea);
		}
		m_targetArea.clearTargetArea();
		m_selectedTile.m_tile = nullptr;
		m_invalidPosition.m_activate = false;
		return;
	}

	//Entity Already Selected whilst showing where to fire
	//Change to different Entity before firing
	if (tileOnMouse->m_entityOnTile && tileOnMouse->m_entityOnTile->m_factionName == m_battle.getCurrentFaction()
		 && m_selectedTile.m_tile && m_targetArea.m_targetArea.size() > 0)
	{
		m_targetArea.clearTargetArea();
		m_targetArea.generateTargetArea(m_battle.getMap(), *tileOnMouse);
		//TODO: Raise info box
		m_selectedTile.m_tile = tileOnMouse;
		return;
	}

	//Click on same
	//Select new Entity to fire at something
	if (tileOnMouse->m_entityOnTile && tileOnMouse->m_entityOnTile->m_factionName != m_battle.getCurrentFaction())
	{
		m_selectedTile.m_tile = tileOnMouse;
		return;
	}

	if (!m_selectedTile.m_tile && tileOnMouse->m_entityOnTile && tileOnMouse->m_entityOnTile->m_factionName == m_battle.getCurrentFaction())
	{
		m_selectedTile.m_tile = tileOnMouse;
	}

	if (m_selectedTile.m_tile && m_selectedTile.m_tile->m_entityOnTile && !m_selectedTile.m_tile->m_entityOnTile->m_battleProperties.isWeaponFired() &&
		m_selectedTile.m_tile->m_entityOnTile->m_factionName == m_battle.getCurrentFaction())
	{
		m_selectedTile.m_tile = tileOnMouse;
		m_targetArea.generateTargetArea(m_battle.getMap(), *tileOnMouse);
	}
}

void BattleUI::onRightClickAttackPhase()
{
	//TODO: Drop info box
	m_selectedTile.m_tile = nullptr;
	m_invalidPosition.m_activate = false;
	m_targetArea.clearTargetArea();
}

void BattleUI::onMouseMoveAttackPhase()
{
	assert(m_battle.getCurrentPhase() == BattlePhase::Attack);
	const Tile* tileOnMouse = m_battle.getMap().getTile(m_battle.getMap().getMouseClickCoord(HAPI_Wrapper::getMouseLocation()));
	if (!tileOnMouse)
	{
		return;
	}

	auto tileCoordinate = tileOnMouse->m_tileCoordinate;
	//tileOnMouse in new position
	if (m_selectedTile.m_tile && m_selectedTile.m_tile->m_tileCoordinate != tileOnMouse->m_tileCoordinate)
	{
		//TODO: this does not work if some of the tiles are nullptr!
		auto cIter = std::find_if(m_targetArea.m_targetArea.cbegin(), m_targetArea.m_targetArea.cend(),
			[tileCoordinate](const auto& tile) { 
				if (tile != nullptr)
				{
					return tileCoordinate == tile->m_tileCoordinate;
				}
			});
		//tileOnMouse within weapon range
		if (cIter != m_targetArea.m_targetArea.cend())
		{
			m_invalidPosition.setPosition(tileOnMouse->m_tileCoordinate, m_battle.getMap());
			m_invalidPosition.m_activate = true;
		}
		//outside weapon range
		else
		{
			m_invalidPosition.m_activate = false;
		}
	}
}


//CurrentSelectedTile m_selectedTile;

//std::pair<int, int> m_leftMouseDownPosition;
////This is used to determine if an entity is currently being given a move command, it gets set to true in the "handleOnLeftClickMovementPhase()" and false after "eLeftMouseButtonUp" is detected.
//bool m_isMovingEntity;
////Used to store the tile selected for movement when the lmb is depressed, so that it can be used for moveEntity input on mouse up
//const Tile* m_mouseDownTile;
//BattleGUI m_gui;
//InvalidPosition m_invalidPosition;
//std::deque<std::unique_ptr<ShipPlacementPhase>> m_playerShipPlacement;

void BattleUI::onResetBattle()
{
	//TODO: reset other things
	m_shipDeployment.clear();
	m_targetArea.onReset();
	m_selectedTile.m_tile = nullptr;
	m_selectedTile.m_position = std::pair<int, int>(0, 0);
	m_invalidPosition.onReset();
	m_leftMouseDownPosition = std::pair<int, int>(0, 0);
	m_mouseDownTile = nullptr;
	m_arrowActive = false;
	m_lastMouseData = { 0,0 };
}

void BattleUI::onNewTurn()
{
	//TODO: Drop info box
	m_selectedTile.m_tile = nullptr;
	m_invalidPosition.m_activate = false;
	clearTargetArea();
	clearSelectedTile();
}

//Weapon Graph
BattleUI::TargetArea::TargetArea()
{
	m_targetAreaSprites.reserve(size_t(400));
	for (int i = 0; i < 400; ++i)
	{
		m_targetAreaSprites.push_back({});
	}
}

void BattleUI::TargetArea::render(const Map& map) const
{
	for (const auto& i : m_targetAreaSprites)
	{
		if (i.activate)
		{
			const std::pair<int, int> tileTransform = map.getTileScreenPos(i.position);

			i.sprite->GetTransformComp().SetPosition({
			static_cast<float>(tileTransform.first + DRAW_ENTITY_OFFSET_X * map.getDrawScale()),
			static_cast<float>(tileTransform.second + DRAW_ENTITY_OFFSET_Y * map.getDrawScale()) });

			i.sprite->Render(SCREEN_SURFACE);
		}
	}
}

void BattleUI::TargetArea::generateTargetArea(const Map & map, const Tile & source, BattlePhase phase)
{
	//TODO: Why isn't this a switch case?
	if (source.m_entityOnTile->m_entityProperties.m_weaponType == eSideCannons)
	{
		m_targetArea = map.cGetTileCone(source.m_tileCoordinate,
			source.m_entityOnTile->m_entityProperties.m_range, 
			source.m_entityOnTile->m_battleProperties.getCurrentDirection(), true);
	
	}

	else if (source.m_entityOnTile->m_entityProperties.m_weaponType == eStraightShot)
	{
		m_targetArea = map.cGetTileLine(source.m_tileCoordinate, 
			source.m_entityOnTile->m_entityProperties.m_range, 
			source.m_entityOnTile->m_battleProperties.getCurrentDirection(), true);

	}

	else if (source.m_entityOnTile->m_entityProperties.m_weaponType == eShotgun)
	{
		// make so where ever the place presses get radius called talk adrais about size of that
		m_targetArea = map.cGetTileRadius(source.m_tileCoordinate, 
			source.m_entityOnTile->m_entityProperties.m_range, true);
	}

	else if (source.m_entityOnTile->m_entityProperties.m_weaponType == eFlamethrower)
	{
		eDirection directionOfFire = eNorth;
		switch (source.m_entityOnTile->m_battleProperties.getCurrentDirection() )
		{
		case eNorth:
			directionOfFire = eSouth;
			break;
		case eNorthEast:
			directionOfFire = eSouthWest;
			break;
		case eSouthEast:
			directionOfFire = eNorthWest;
			break;
		case eSouth:
			directionOfFire = eNorth;
			break;
		case eSouthWest:
			directionOfFire = eNorthEast;
			break;
		case eNorthWest:
			directionOfFire = eSouthEast;
			break;
		}
		m_targetArea = map.cGetTileLine(source.m_tileCoordinate,
			source.m_entityOnTile->m_entityProperties.m_range, directionOfFire, true);
	}
	
	if (m_targetArea.empty())
	{
		return;
	}
	//clearTargetArea();
	assert(!m_targetAreaSprites.empty());
	//using same convention as movement // from source should be able to get position
	for (int i = 0; i < m_targetArea.size(); i++)
	{
		if (!m_targetArea[i])//Check that i is not nullptr
			continue;

		std::pair<int, int>tilePos = map.getTileScreenPos(m_targetArea[i]->m_tileCoordinate);
		m_targetAreaSprites[i].sprite->GetTransformComp().SetPosition(
			{
				 tilePos.first + 12 * map.getDrawScale(),
				 tilePos.second + 28 * map.getDrawScale()
			});

		m_targetAreaSprites[i].activate = true;
		m_targetAreaSprites[i].position = m_targetArea[i]->m_tileCoordinate;
	}
}

void BattleUI::TargetArea::clearTargetArea()
{
	for (auto& i : m_targetAreaSprites)
	{
		i.activate = false;
	}
}

void BattleUI::TargetArea::onReset()
{
	//m_targetAreaSprites.clear();
	m_targetArea.clear();
}

BattleUI::TargetArea::HighlightNode::HighlightNode()
	: sprite(std::make_unique<Sprite>(Textures::m_mouseCrossHair)),
	activate(false)
{
	sprite->GetTransformComp().SetOriginToCentreOfFrame();
	sprite->GetTransformComp().SetScaling({ 0.75f, 0.75f });
}

BattleUI::DeploymentPhase::DeploymentPhase(std::vector<EntityProperties*> player, 
	std::pair<int, int> spawnPosition, int range, const Map& map, FactionName factionName)
	: m_factionName(factionName),
	m_player(player),
	m_currentSelectedEntity(),
	m_spawnArea(),
	m_spawnSprites(),
	m_spawnPosition(spawnPosition)
{
	//Might change this - for now its two containers but looks confusing
	m_spawnArea = map.cGetTileRadius(spawnPosition, range, true, true);
	m_spawnSprites.reserve(m_spawnArea.size());
	for (int i = 0; i < m_spawnArea.size(); ++i)
	{
		std::unique_ptr<Sprite> sprite;
		switch (factionName)
		{
		case eYellow:
			sprite = HAPI_Sprites.MakeSprite(Textures::m_yellowSpawnHex);
			break;
		case eBlue:
			sprite = HAPI_Sprites.MakeSprite(Textures::m_blueSpawnHex);
			break;
		case eGreen:
			sprite = HAPI_Sprites.MakeSprite(Textures::m_greenSpawnHex);
			break;
		case eRed:
			sprite = HAPI_Sprites.MakeSprite(Textures::m_redSpawnHex);
			break;
		};

		auto screenPosition = map.getTileScreenPos(m_spawnArea[i]->m_tileCoordinate);
		sprite->GetTransformComp().SetPosition({
			(float)screenPosition.first + DRAW_ENTITY_OFFSET_X * map.getDrawScale(),
			(float)screenPosition.second + DRAW_ENTITY_OFFSET_Y * map.getDrawScale() });
		sprite->GetTransformComp().SetOriginToCentreOfFrame();
		sprite->GetTransformComp().SetScaling({ 2.f, 2.f });

		m_spawnSprites.push_back(std::move(sprite));
	}
	/*
	for (int i = 0; i < m_spawnArea.size(); ++i)
	{
		//const std::pair<int, int> tileTransform = map.getTileScreenPos(m_tile->m_tileCoordinate);

		//m_sprite->GetTransformComp().SetPosition({
		//static_cast<float>(tileTransform.first + DRAW_ENTITY_OFFSET_X * map.getDrawScale()),
		//static_cast<float>(tileTransform.second + DRAW_ENTITY_OFFSET_Y * map.getDrawScale()) });

		auto screenPosition = map.getTileScreenPos(m_spawnArea[i]->m_tileCoordinate);
		m_spawnSprites[i]->GetTransformComp().SetPosition({
			(float)screenPosition.first + DRAW_ENTITY_OFFSET_X * map.getDrawScale(),
			(float)screenPosition.second + DRAW_ENTITY_OFFSET_Y * map.getDrawScale() });
		m_spawnSprites[i]->GetTransformComp().SetOriginToCentreOfFrame();
		m_spawnSprites[i]->GetTransformComp().SetScaling({ 2.f, 2.f });
	}*/

	m_currentSelectedEntity.m_currentSelectedEntity = m_player.back();
}

bool BattleUI::DeploymentPhase::isCompleted() const
{
	return m_player.empty();
}

void BattleUI::DeploymentPhase::render(const InvalidPosition& invalidPosition, const Map& map) const
{
	if (m_currentSelectedEntity.m_currentSelectedEntity && !invalidPosition.m_activate)
	{
		const std::pair<int, int> tileTransform = map.getTileScreenPos(m_currentSelectedEntity.m_position);

		m_currentSelectedEntity.m_currentSelectedEntity->m_sprite->GetTransformComp().SetPosition({
		static_cast<float>(tileTransform.first + DRAW_ENTITY_OFFSET_X * map.getDrawScale()),
		static_cast<float>(tileTransform.second + DRAW_ENTITY_OFFSET_Y * map.getDrawScale()) });

		m_currentSelectedEntity.m_currentSelectedEntity->m_sprite->Render(SCREEN_SURFACE);
	}

	for (int i = 0; i < m_spawnSprites.size(); ++i)
	{
		const std::pair<int, int> tileTransform = map.getTileScreenPos(m_spawnArea[i]->m_tileCoordinate);

		m_spawnSprites[i]->GetTransformComp().SetPosition({
		static_cast<float>(tileTransform.first + DRAW_ENTITY_OFFSET_X * map.getDrawScale()),
		static_cast<float>(tileTransform.second + DRAW_ENTITY_OFFSET_Y * map.getDrawScale()) });

		m_spawnSprites[i]->Render(SCREEN_SURFACE);
	}
}

void BattleUI::DeploymentPhase::onReset()
{
	m_player.clear();
	m_currentSelectedEntity.m_position = std::pair<int, int>(0, 0);
	m_currentSelectedEntity.m_currentSelectedEntity = nullptr;
	m_spawnArea.clear();
	m_spawnSprites.clear();
	m_spawnPosition = std::pair<int, int>(0, 0);
}

const Tile* BattleUI::DeploymentPhase::getTileOnMouse(InvalidPosition& invalidPosition, const Tile* currentTileSelected, const Map& map)
{
	const Tile* tileOnMouse = map.getTile(map.getMouseClickCoord(HAPI_Wrapper::getMouseLocation()));
	if (!tileOnMouse)
	{
		return nullptr;
	}

	if (!currentTileSelected)
	{
		return tileOnMouse;
	}

	//If new tile is in new position than old tile
	if (currentTileSelected->m_tileCoordinate != tileOnMouse->m_tileCoordinate)
	{
		currentTileSelected = tileOnMouse;
		
		//Cannot place ship on existing ship
		if (tileOnMouse->m_entityOnTile)
		{
			invalidPosition.setPosition(map.getTileScreenPos(tileOnMouse->m_tileCoordinate), map);
			invalidPosition.m_activate = true;
			return tileOnMouse;
		}

		auto tileCoordinate = tileOnMouse->m_tileCoordinate;
		auto iter = std::find_if(m_spawnArea.begin(), m_spawnArea.end(), [tileCoordinate](const auto& tile) { return tileCoordinate == tile->m_tileCoordinate; });
		if (iter != m_spawnArea.cend())
		{
			const std::pair<int, int> tileTransform = map.getTileScreenPos(tileOnMouse->m_tileCoordinate);
			
			m_currentSelectedEntity.m_currentSelectedEntity->m_sprite->GetTransformComp().SetPosition({
				static_cast<float>(tileTransform.first + DRAW_ENTITY_OFFSET_X * map.getDrawScale()),
				static_cast<float>(tileTransform.second + DRAW_ENTITY_OFFSET_Y * map.getDrawScale()) });
			m_currentSelectedEntity.m_position = tileOnMouse->m_tileCoordinate;

			invalidPosition.m_activate = false;
		}
		else
		{
			invalidPosition.setPosition(tileOnMouse->m_tileCoordinate, map);
			invalidPosition.m_activate = true;
		}
	}

	return tileOnMouse;
}

void BattleUI::DeploymentPhase::onLeftClick(InvalidPosition& invalidPosition, eDirection startingDirection, const Tile* currentTileSelected, Battle& battle)
{
	if (!currentTileSelected)
	{
		return;
	}
	//Disallow spawning on land
	if (currentTileSelected->m_type != eTileType::eSea && currentTileSelected->m_type != eTileType::eOcean)
	{
		return;
	}
	if (!invalidPosition.m_activate && !currentTileSelected->m_entityOnTile)
	{
		battle.insertEntity(currentTileSelected->m_tileCoordinate, startingDirection, *m_currentSelectedEntity.m_currentSelectedEntity, m_factionName);
		//invalidPosition.m_activate = true;
		invalidPosition.m_position = currentTileSelected->m_tileCoordinate;
		//Change ordering around to pop front with different container
		m_player.pop_back();
		if (m_player.empty())
		{
			battle.nextTurn();
		}
		else
		{
			m_currentSelectedEntity.m_currentSelectedEntity = m_player.back();
		}
	}
}

//Current Selected Tile
BattleUI::CurrentSelectedTile::CurrentSelectedTile()
	: m_sprite(std::make_unique<Sprite>(Textures::m_selectedHex)),
	m_tile(nullptr),
	m_position()
{
	m_sprite->GetTransformComp().SetOriginToCentreOfFrame();
	m_sprite->GetTransformComp().SetScaling({ 1.9f, 1.9f });
}

void BattleUI::CurrentSelectedTile::render(const Map & map) const
{
	if (m_tile && (m_tile->m_type == eTileType::eSea || m_tile->m_type == eTileType::eOcean))
	{
		const std::pair<int, int> tileTransform = map.getTileScreenPos(m_tile->m_tileCoordinate);

		m_sprite->GetTransformComp().SetPosition({
		static_cast<float>(tileTransform.first + DRAW_ENTITY_OFFSET_X * map.getDrawScale()),
		static_cast<float>(tileTransform.second + DRAW_ENTITY_OFFSET_Y * map.getDrawScale()) });

		m_sprite->Render(SCREEN_SURFACE);
	}
}

std::pair<int, int> BattleUI::DeploymentPhase::getSpawnPosition() const
{
	return m_spawnPosition;
}