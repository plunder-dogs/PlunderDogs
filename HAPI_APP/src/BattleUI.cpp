#include "BattleUI.h"
#include "Battle.h"
#include "BFS.h"
#include "Textures.h"
#include "MouseSelection.h"
#include "GameEventMessenger.h"
#include <assert.h>
#include "AI.h"

using namespace HAPISPACE;
constexpr int SHIP_PLACEMENT_SPAWN_RANGE{ 3 };
constexpr int MAX_MOVE_AREA{ 700 };

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
		static_cast<float>(tileTransform.first + DRAW_OFFSET_X * map.getDrawScale()),
		static_cast<float>(tileTransform.second + DRAW_OFFSET_Y * map.getDrawScale()) });

		m_sprite->Render(SCREEN_SURFACE);
	}
}

void BattleUI::InvalidPosition::setPosition(std::pair<int, int> newPosition, const Map& map)
{
	m_sprite->GetTransformComp().SetPosition({
		(float)newPosition.first + DRAW_OFFSET_X * map.getDrawScale(),
		(float)newPosition.second + DRAW_OFFSET_Y * map.getDrawScale()});

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
	m_lastMouseData({0, 0}),
	m_targetArea(),
	m_movementArea(Textures::m_selectedHex, m_battle.getMap())
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

void BattleUI::renderUI() const
{
	switch (m_battle.getCurrentPhase())
	{
	case BattlePhase::Deployment:
		renderArrow();
		break;

	case BattlePhase::Movement:
		m_selectedTile.render(m_battle.getMap());
		m_movementArea.render(m_battle.getMap());
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
	//m_gui.render(m_battle.getCurrentPhase());

	if (m_selectedTile.m_tile != nullptr && m_selectedTile.m_tile->m_shipOnTile.isValid())
	{
		//m_gui.renderStats(m_battle.getFactionShip(m_selectedTile.m_tile->m_shipOnTile));
	}
}

void BattleUI::loadGUI(std::pair<int, int> mapDimensions)
{
	m_gui.setMaxCameraOffset(mapDimensions);
}

void BattleUI::onFactionWin(FactionName winningFaction)
{
	switch (winningFaction)
	{
	case FactionName::eYellow :
		m_gui.onYellowWin();
		break;
	case FactionName::eBlue :
		m_gui.onBlueWin();
		break;
	case FactionName::eGreen :
		m_gui.onGreenWin();
		break;
	case FactionName::eRed :
		m_gui.onRedWin();
		break;
	}
}

void BattleUI::onEnteringBattlePhase(BattlePhase currentBattlePhase)
{
	switch (currentBattlePhase)
	{
	case BattlePhase::Movement :
		m_gui.onEnteringMovementPhase();
		break;
	case BattlePhase::Attack:
		m_gui.onEnteringAttackPhase();
		break;
	}
}

void BattleUI::drawTargetArea() const
{
	for (const auto& i : m_targetArea.m_targetAreaSprites)
	{
		if (i.activate)
		{
			const std::pair<int, int> tileTransform = m_battle.getMap().getTileScreenPos(i.position);

			i.sprite->GetTransformComp().SetPosition({
			static_cast<float>(tileTransform.first + DRAW_OFFSET_X * m_battle.getMap().getDrawScale()),
			static_cast<float>(tileTransform.second + DRAW_OFFSET_Y * m_battle.getMap().getDrawScale()) });

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

void BattleUI::OnMouseEvent(EMouseEvent mouseEvent, const HAPI_TMouseData & mouseData)
{
	m_lastMouseData = { mouseData.x , mouseData.y };

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
				if (!m_selectedTile.m_tile)
					break;
				if (mouseMoveDirection.first > 20)
				{
					onLeftClickDeploymentPhase(mouseMoveDirection.second);
					m_selectedTile.m_tile = nullptr;
					m_invalidPosition.m_activate = true;
				}
				else
				{
					//This function call is to be used if the movement of the mouse during the move command is small enough to be considered unintended,
					//in this case the ship should not rotate after reaching the destination.
					onLeftClickDeploymentPhase();
					m_selectedTile.m_tile = nullptr;

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
					m_battle.moveFactionShipToPosition(m_selectedTile.m_tile->m_shipOnTile, m_mouseDownTile->m_tileCoordinate, mouseMoveDirection.second);
				}
				else
				{
					m_battle.moveFactionShipToPosition(m_selectedTile.m_tile->m_shipOnTile, m_mouseDownTile->m_tileCoordinate);
				}
				m_arrowActive = false;
				m_movementArea.clear();
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


	m_gui.OnMouseMove(mouseData, m_battle.getCurrentPhase());

	switch (m_battle.getCurrentPhase())
	{
	case BattlePhase::Deployment:
	{
		if (!m_isMovingEntity)
		{
			onMouseMoveDeploymentPhase();
			//m_selectedTile.m_tile = m_shipDeployment.front()->getTileOnMouse(
			//	m_invalidPosition, m_selectedTile.m_tile, m_battle.getMap());
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
	if (m_selectedTile.m_tile && m_selectedTile.m_tile->m_shipOnTile.isValid())
	{
		m_battle.disableFactionShipMovementPath(m_selectedTile.m_tile->m_shipOnTile);
	}
		
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

void BattleUI::onMouseMoveDeploymentPhase()
{
	if (m_battle.getCurrentPlayerType() == ePlayerType::eAI)
	{
		return;
	}


	const Map& map = m_battle.getMap();
	const Tile* tileOnMouse = map.getTile(map.getMouseClickCoord(HAPI_Wrapper::getMouseLocation()));
	if (!tileOnMouse)
	{
		return;
	}

	if (!m_selectedTile.m_tile)
	{
		m_selectedTile.m_tile = tileOnMouse;
		return;
	}

	//If new tile is in new position than old tile
	if (m_selectedTile.m_tile->m_tileCoordinate != tileOnMouse->m_tileCoordinate)
	{
		m_selectedTile.m_tile = tileOnMouse;

		//Cannot place ship on existing ship
		if (tileOnMouse->m_shipOnTile.isValid())
		{
			m_invalidPosition.setPosition(map.getTileScreenPos(tileOnMouse->m_tileCoordinate), map);
			m_invalidPosition.m_activate = true;
		}
		
		//If within spawn area
		if (m_battle.setShipDeploymentAtPosition(tileOnMouse->m_tileCoordinate))
		{
			m_invalidPosition.m_activate = false;
		}
		//Outside spawn area
		else
		{
			m_invalidPosition.setPosition(tileOnMouse->m_tileCoordinate, map);
			m_invalidPosition.m_activate = true;
		}
	}
}

void BattleUI::onLeftClickDeploymentPhase(eDirection startingDirection)
{
	if (m_battle.getCurrentPlayerType() == ePlayerType::eAI)
	{
		return;
	}

	if (m_selectedTile.m_tile && (m_selectedTile.m_tile->m_type == eTileType::eSea || m_selectedTile.m_tile->m_type == eTileType::eOcean))
	{
		if (!m_invalidPosition.m_activate && !m_selectedTile.m_tile->m_shipOnTile.isValid())
		{
			m_invalidPosition.m_position = m_selectedTile.m_tile->m_tileCoordinate;
			m_battle.deployFactionShipAtPosition(m_selectedTile.m_tile->m_tileCoordinate, startingDirection);
		}
	}
}

void BattleUI::onMouseMoveMovementPhase()
{
	assert(m_battle.getCurrentPhase() == BattlePhase::Movement);

	if (m_battle.getCurrentPlayerType() == ePlayerType::eAI)
	{
		return;
	}

	//Current tile selected does not match the current player in play
	if (m_selectedTile.m_tile && m_selectedTile.m_tile->m_shipOnTile.isValid()
		&& m_battle.getFactionShip(m_selectedTile.m_tile->m_shipOnTile).getFactionName() != m_battle.getCurrentFaction())
	{
		return;
	}

	//!isMovingToDestination && !reachedDestination
	if (m_selectedTile.m_tile && m_selectedTile.m_tile->m_shipOnTile.isValid() &&  
		!m_battle.getFactionShip(m_selectedTile.m_tile->m_shipOnTile).isMovingToDestination() &&
		!m_battle.getFactionShip(m_selectedTile.m_tile->m_shipOnTile).isDestinationSet())
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
				m_battle.disableFactionShipMovementPath(m_selectedTile.m_tile->m_shipOnTile);
			}
			else
			{
				m_battle.generateFactionShipMovementPath(m_selectedTile.m_tile->m_shipOnTile, tile->m_tileCoordinate);
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

	if (!m_selectedTile.m_tile && tileOnMouse->m_shipOnTile.isValid())
	{
		m_selectedTile.m_tile = tileOnMouse;
		if (m_battle.getCurrentPlayerType() != ePlayerType::eAI)
		{
			m_battle.disableFactionShipMovementPath(m_selectedTile.m_tile->m_shipOnTile);
			//Display the available movement tiles
			const Ship& ship = m_battle.getFactionShip(m_selectedTile.m_tile->m_shipOnTile);
			if (!ship.isDestinationSet())
			{
				m_movementArea.newArea(m_battle.getMap(), ship);
			}
		}
		return;
	}

	if (m_battle.getCurrentPlayerType() == ePlayerType::eAI)
	{
		return;
	}

	if (!m_selectedTile.m_tile && tileOnMouse->m_shipOnTile.isValid() &&
		m_battle.getFactionShip(tileOnMouse->m_shipOnTile).getFactionName() != m_battle.getCurrentFaction())
	{
		m_selectedTile.m_tile = tileOnMouse;
		return;
	}

	//Invalid Location - Collidable tile
	if (tileOnMouse->m_type != eTileType::eSea && tileOnMouse->m_type != eTileType::eOcean)
	{
		if (m_selectedTile.m_tile && m_selectedTile.m_tile->m_shipOnTile.isValid())
		{
			m_battle.disableFactionShipMovementPath(m_selectedTile.m_tile->m_shipOnTile); 
		}
		return;
	}

	//Do not select killed entity
	if (tileOnMouse->m_shipOnTile.isValid() && m_battle.getFactionShip(tileOnMouse->m_shipOnTile).isDead())
	{
		//TODO: Drop info box
		m_selectedTile.m_tile = nullptr;
		m_movementArea.clear();
		return;
	}

	//Clicking to where entity is moving to
	if (tileOnMouse->m_shipOnTile.isValid() && m_battle.getFactionShip(tileOnMouse->m_shipOnTile).isDestinationSet())
	{
		//TODO: Drop info box
		m_selectedTile.m_tile = nullptr;
		m_movementArea.clear();
		return;
	}

	if (m_selectedTile.m_tile)
	{
		//Cancel movement if clicked on same entity
		if (m_selectedTile.m_tile->m_tileCoordinate == tileOnMouse->m_tileCoordinate)
		{
			m_battle.disableFactionShipMovementPath(m_selectedTile.m_tile->m_shipOnTile);
			//TODO: Drop info box
			m_selectedTile.m_tile = nullptr;
		}

		//Disallow movement to tile occupied by other player
		else if (tileOnMouse->m_shipOnTile.isValid())
		{
			m_battle.disableFactionShipMovementPath(m_selectedTile.m_tile->m_shipOnTile); 
			m_selectedTile.m_tile = tileOnMouse;

			const Ship& ship = m_battle.getFactionShip(m_selectedTile.m_tile->m_shipOnTile);
			if (ship.getFactionName() == m_battle.getCurrentFaction() && !ship.isDestinationSet())
			{
				m_movementArea.newArea(m_battle.getMap(), ship);
			}
		}

		//Store data so Entity can move to new location
		else if (m_selectedTile.m_tile->m_shipOnTile.isValid() && (m_selectedTile.m_tile->m_tileCoordinate != tileOnMouse->m_tileCoordinate)
			&& m_battle.getFactionShip(m_selectedTile.m_tile->m_shipOnTile).getFactionName() == m_battle.getCurrentFaction())
		{
			assert(m_battle.getFactionShip(m_selectedTile.m_tile->m_shipOnTile).getFactionName() == m_battle.getCurrentFaction());
			m_mouseDownTile = tileOnMouse;
			m_isMovingEntity = true;
			auto test = m_battle.getFactionShip(m_selectedTile.m_tile->m_shipOnTile).getEndOfMovementPath();
			if (m_battle.getFactionShip(m_selectedTile.m_tile->m_shipOnTile).getEndOfMovementPath() == tileOnMouse->m_tileCoordinate)
				m_arrowActive = true;

			m_battle.moveFactionShipToPosition(m_selectedTile.m_tile->m_shipOnTile, tileOnMouse->m_tileCoordinate);
			m_selectedTile.m_tile = nullptr;
		}
	}
	else
	{
		if (tileOnMouse->m_shipOnTile.isValid() && m_battle.getFactionShip(tileOnMouse->m_shipOnTile).isDead())
		{
			//TODO: Drop info box
			m_selectedTile.m_tile = nullptr;
			m_movementArea.clear();
		}
		//Do not select tile that contains wrong players entity
		if (tileOnMouse->m_shipOnTile.isValid())
		{
			if (m_battle.getFactionShip(tileOnMouse->m_shipOnTile).getFactionName() != m_battle.getCurrentFaction())
			{
				//TODO: Drop info box
				m_selectedTile.m_tile = nullptr;
				m_movementArea.clear();
			}
			//Selecting another ship you own
			else
			{
				m_selectedTile.m_tile = tileOnMouse;
			}
		}
	}
}

void BattleUI::onRightClickMovementPhase()
{
	assert(m_battle.getCurrentPhase() == BattlePhase::Movement);
	//Cancel selected Entity
	if (m_selectedTile.m_tile && m_selectedTile.m_tile->m_shipOnTile.isValid())
	{
		m_battle.disableFactionShipMovementPath(m_selectedTile.m_tile->m_shipOnTile); 
		m_invalidPosition.m_activate = false;
	}
	//TODO: Drop info box
	m_selectedTile.m_tile = nullptr;
	m_movementArea.clear();
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

	if (m_battle.getCurrentPlayerType() == ePlayerType::eAI)
	{
		m_selectedTile.m_tile = tileOnMouse;
		return;
	}

	//Select new entity that is on same team
	if (m_selectedTile.m_tile && m_selectedTile.m_tile->m_shipOnTile.isValid() && tileOnMouse->m_shipOnTile.isValid() &&
		(m_battle.getFactionShip(tileOnMouse->m_shipOnTile).getFactionName() == m_battle.getCurrentFaction()))
	{
		if (!m_battle.getFactionShip(tileOnMouse->m_shipOnTile).isWeaponFired())
		{
			m_targetArea.clearTargetArea();
			m_targetArea.generateTargetArea(m_battle, *tileOnMouse);
			m_selectedTile.m_tile = tileOnMouse;
			return;
		}
	}

	if (!m_selectedTile.m_tile && tileOnMouse->m_shipOnTile.isValid() && m_battle.getFactionShip(tileOnMouse->m_shipOnTile).getFactionName() != m_battle.getCurrentFaction())
	{
		m_selectedTile.m_tile = tileOnMouse;
		return;
	}

	//Do not fire on destroyed ship
	if (tileOnMouse->m_shipOnTile.isValid() && m_battle.getFactionShip(tileOnMouse->m_shipOnTile).isDead())
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
	if (m_selectedTile.m_tile && m_selectedTile.m_tile->m_shipOnTile.isValid() && 
		!m_battle.getFactionShip(m_selectedTile.m_tile->m_shipOnTile).isWeaponFired())
	{
		//If there is an entity on the tile you're clicking on and that entity's faction name differs from the one of the ship that's firing
		if ((tileOnMouse->m_shipOnTile.isValid()) && m_battle.getFactionShip(tileOnMouse->m_shipOnTile).getFactionName() != 
			m_battle.getFactionShip(m_selectedTile.m_tile->m_shipOnTile).getFactionName())
		{
			m_battle.fireFactionShipAtPosition(m_selectedTile.m_tile->m_shipOnTile, tileOnMouse->m_shipOnTile, m_targetArea.m_targetArea);
		}
		m_targetArea.clearTargetArea();
		m_selectedTile.m_tile = nullptr;
		m_invalidPosition.m_activate = false;
		return;
	}

	//Entity Already Selected whilst showing where to fire
	//Change to different Entity before firing
	if (tileOnMouse->m_shipOnTile.isValid() && m_battle.getFactionShip(tileOnMouse->m_shipOnTile).getFactionName() == m_battle.getCurrentFaction()
		 && m_selectedTile.m_tile && m_targetArea.m_targetArea.size() > 0)
	{
		m_targetArea.clearTargetArea();
		m_targetArea.generateTargetArea(m_battle, *tileOnMouse);
		//TODO: Raise info box
		m_selectedTile.m_tile = tileOnMouse;
		return;
	}

	//Click on same
	//Select new Entity to fire at something
	if (tileOnMouse->m_shipOnTile.isValid() && m_battle.getFactionShip(tileOnMouse->m_shipOnTile).getFactionName() != m_battle.getCurrentFaction())
	{
		m_selectedTile.m_tile = tileOnMouse;
		return;
	}

	if (!m_selectedTile.m_tile && tileOnMouse->m_shipOnTile.isValid() && m_battle.getFactionShip(tileOnMouse->m_shipOnTile).getFactionName() == 
		m_battle.getCurrentFaction())
	{
		m_selectedTile.m_tile = tileOnMouse;
	}

	if (m_selectedTile.m_tile && m_selectedTile.m_tile->m_shipOnTile.isValid() && 
		!m_battle.getFactionShip(m_selectedTile.m_tile->m_shipOnTile).isWeaponFired() &&
		m_battle.getFactionShip(m_selectedTile.m_tile->m_shipOnTile).getFactionName() == m_battle.getCurrentFaction())
	{
		m_selectedTile.m_tile = tileOnMouse;
		m_targetArea.generateTargetArea(m_battle, *tileOnMouse);
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

void BattleUI::onResetBattle()
{
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
	m_selectedTile.m_tile = nullptr;
	m_invalidPosition.m_activate = false;
	clearTargetArea();
	clearSelectedTile();
	m_movementArea.clear();
}

//Weapon Graph
BattleUI::TargetArea::TargetArea()
{
	m_targetAreaSprites.reserve(size_t(50));
	for (int i = 0; i < 50; ++i)
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
			static_cast<float>(tileTransform.first + DRAW_OFFSET_X * map.getDrawScale()),
			static_cast<float>(tileTransform.second + DRAW_OFFSET_Y * map.getDrawScale()) });

			i.sprite->Render(SCREEN_SURFACE);
		}
	}
}

void BattleUI::TargetArea::generateTargetArea(Battle& battle, const Tile & source, BattlePhase phase)
{
	const Ship& ship = battle.getFactionShip(source.m_shipOnTile);
	if (ship.getShipType() == eShipType::eFrigate)
	{
		m_targetArea = battle.getMap().cGetTileCone(source.m_tileCoordinate,
			ship.getRange(), 
			ship.getCurrentDirection(), true);
	
	}
	else if (ship.getShipType() == eShipType::eSniper)
	{
		m_targetArea = battle.getMap().cGetTileLine(source.m_tileCoordinate, 
			ship.getRange(),
			ship.getCurrentDirection(), true);

	}
	else if (ship.getShipType() == eShipType::eTurtle)
	{
		// make so where ever the place presses get radius called talk adrais about size of that
		m_targetArea = battle.getMap().cGetTileRadius(source.m_tileCoordinate, 
			ship.getRange(), true);
	}
	else if (ship.getShipType() == eShipType::eFire)
	{
		eDirection directionOfFire = eNorth;
		switch (ship.getCurrentDirection() )
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
		m_targetArea = battle.getMap().cGetTileLine(source.m_tileCoordinate,
			ship.getRange(), directionOfFire, true);
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

		std::pair<int, int>tilePos = battle.getMap().getTileScreenPos(m_targetArea[i]->m_tileCoordinate);
		m_targetAreaSprites[i].sprite->GetTransformComp().SetPosition(
			{
				 tilePos.first + 12 * battle.getMap().getDrawScale(),
				 tilePos.second + 28 * battle.getMap().getDrawScale()
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

//Current Selected Tile
BattleUI::SelectedTile::SelectedTile()
	: m_sprite(std::make_unique<Sprite>(Textures::m_selectedHex)),
	m_tile(nullptr),
	m_position()
{
	m_sprite->GetTransformComp().SetOriginToCentreOfFrame();
	m_sprite->GetTransformComp().SetScaling({ 1.9f, 1.9f });
}

void BattleUI::SelectedTile::render(const Map & map) const
{
	if (m_tile && (m_tile->m_type == eTileType::eSea || m_tile->m_type == eTileType::eOcean))
	{
		const std::pair<int, int> tileTransform = map.getTileScreenPos(m_tile->m_tileCoordinate);

		m_sprite->GetTransformComp().SetPosition({
		static_cast<float>(tileTransform.first + DRAW_OFFSET_X * map.getDrawScale()),
		static_cast<float>(tileTransform.second + DRAW_OFFSET_Y * map.getDrawScale()) });

		m_sprite->Render(SCREEN_SURFACE);
	}

}


BattleUI::MovementArea::MovementArea(std::shared_ptr<HAPISPACE::SpriteSheet>& texturePtr, const Map& map)
	: m_display(false),
	m_displaySize(0),
	m_tileOverlays()
{
	m_tileOverlays.resize(MAX_MOVE_AREA);
	float scale = map.getDrawScale();
	for (int i = 0; i < MAX_MOVE_AREA; i++)
	{
		m_tileOverlays[i].second = std::make_unique<HAPISPACE::Sprite>(texturePtr);
		m_tileOverlays[i].second->GetTransformComp().SetOriginToCentreOfFrame();
		m_tileOverlays[i].second->GetTransformComp().SetScaling({ scale, scale });
	}
}

void BattleUI::MovementArea::render(const Map& map) const
{
	if (m_display)
	{
		float scale = map.getDrawScale();
		for (int i = 0; i < m_displaySize; i++)
		{
			posi pos = map.getTileScreenPos(m_tileOverlays[i].first);
			float x = static_cast<float>(pos.x) + DRAW_OFFSET_X * scale;
			float y = static_cast<float>(pos.y) + DRAW_OFFSET_Y * scale;
			m_tileOverlays[i].second->GetTransformComp().SetPosition({ x, y });
			m_tileOverlays[i].second->Render(SCREEN_SURFACE);
		}
	}
}

void BattleUI::MovementArea::newArea(const Map& map, const Ship& ship)
{
	posi startPos = { ship.getCurrentPosition(), ship.getCurrentDirection() };
	std::vector<posi> area = BFS::findArea(map, startPos, ship.getMovementPoints());
	
	m_displaySize = area.size();
	for (int i = 0; i < m_displaySize; i++)
	{
		m_tileOverlays[i].first = area[i].pair();
	}

	m_display = true;
}

void BattleUI::MovementArea::clear()
{
	m_display = false;
	m_displaySize = 0;
}