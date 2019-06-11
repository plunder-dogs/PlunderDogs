#include "BattleUI.h"
#include "Battle.h"
#include "BFS.h"
#include "Textures.h"
#include "MouseSelection.h"
#include "GameEventMessenger.h"
#include <assert.h>
#include "AI.h"

#include <iostream>

constexpr int SHIP_PLACEMENT_SPAWN_RANGE{ 3 };

//
//BattleUI
//
BattleUI::BattleUI(Battle & battle)
	: m_battle(battle),
	m_tileOnPreviousClick(nullptr),
	m_gui(),
	m_tileOnClick(nullptr),
	m_tileOnMouse(nullptr),
	m_leftMouseDownPosition({ 0, 0 }),
	m_isMovingEntity(false),
	m_mouseDownTile(nullptr),
	m_arrowActive(false),
	m_arrowSprite(),
	m_lastMouseData({0, 0}),
	m_targetArea(),
	m_movementArea(Textures::getInstance().m_selectedHex, m_battle.getMap())
{
	m_arrowSprite.setScale({ 0.5, 0.5 });
	m_tileHighlight.setScale(sf::Vector2f(1.9f, 1.9f));
	//m_arrowSprite->GetTransformComp().SetOriginToCentreOfFrame();
	GameEventMessenger::getInstance().subscribe(std::bind(&BattleUI::onResetBattle, this), "BattleUI", GameEvent::eResetBattle);
	GameEventMessenger::getInstance().subscribe(std::bind(&BattleUI::onNewTurn, this), "BattleUI", GameEvent::eNewTurn);
}

BattleUI::~BattleUI()
{
	GameEventMessenger::getInstance().unsubscribe("BattleUI", GameEvent::eResetBattle);
	GameEventMessenger::getInstance().unsubscribe("BattleUI", GameEvent::eNewTurn);
}

sf::Vector2i BattleUI::getCameraPositionOffset() const
{
	return m_gui.getCameraPositionOffset();
}

void BattleUI::renderUI(sf::RenderWindow& window)
{
	switch (m_battle.getCurrentPhase())
	{
	case BattlePhase::Deployment:
		//renderArrow(window);
		break;

	case BattlePhase::Movement:
		renderTileHighlight(window);
		m_movementArea.render(window, m_battle.getMap());
		//renderArrow();
		break;
	
	case BattlePhase::Attack:
		renderTileHighlight(window);
		break;
	}
}

void BattleUI::renderGUI(sf::RenderWindow& window)
{
	m_mouseSprite.render(window, m_battle.getMap());
	//m_gui.render(m_battle.getCurrentPhase());

	if (m_tileOnClick != nullptr && m_tileOnClick->m_shipOnTile.isValid())
	{
		//m_gui.renderStats(m_battle.getFactionShip(m_selectedTile.m_tile->m_shipOnTile));
	}
}

void BattleUI::loadGUI(sf::Vector2i mapDimensions)
{
	m_gui.setMaxCameraOffset(mapDimensions);
}

void BattleUI::onFactionWin(FactionName winningFaction)
{
	switch (winningFaction)
	{
	//case FactionName::eYellow :
	//	m_gui.onYellowWin();
	//	break;
	//case FactionName::eBlue :
	//	m_gui.onBlueWin();
	//	break;
	//case FactionName::eGreen :
	//	m_gui.onGreenWin();
	//	break;
	//case FactionName::eRed :
	//	m_gui.onRedWin();
	//	break;
	}
}

void BattleUI::onEnteringBattlePhase(BattlePhase currentBattlePhase)
{
	switch (currentBattlePhase)
	{
	case BattlePhase::Movement :
		//m_gui.onEnteringMovementPhase();
		break;
	case BattlePhase::Attack:
		//m_gui.onEnteringAttackPhase();
		break;
	}
}

void BattleUI::drawTargetArea(sf::RenderWindow& window)
{
	for (auto& i : m_targetArea.m_targetAreaGraph)
	{
		if (i.isActive())
		{
			i.render(window, m_battle.getMap());

			//const sf::Vector2i tileTransform = m_battle.getMap().getTileScreenPos(i..position);

			//i.sprite.setPosition({
			//static_cast<float>(tileTransform.x + DRAW_OFFSET_X * m_battle.getMap().getDrawScale()),
			//static_cast<float>(tileTransform.y + DRAW_OFFSET_Y * m_battle.getMap().getDrawScale()) });

			//window.draw(i.sprite);
		}
	}
}

void BattleUI::handleInput(sf::RenderWindow& window, const sf::Event & currentEvent)
{
	m_lastMouseData = sf::Mouse::getPosition(window);

	if (currentEvent.type == sf::Event::MouseButtonPressed)
	{
		if (currentEvent.mouseButton.button == sf::Mouse::Left)
		{
			if (m_battle.getCurrentPlayerType() == ePlayerType::eAI)
			{
				return;
			}

			onLeftClick(window);
		}
		else if (currentEvent.mouseButton.button == sf::Mouse::Right)
		{
			switch (m_battle.getCurrentPhase())
			{
			case BattlePhase::Movement:
			{
				onRightClickMovementPhase(m_lastMouseData);
				break;
			}
			case BattlePhase::Attack:
			{
				onRightClickAttackPhase(m_lastMouseData);
				break;
			}
			} 
		}
	}
	else if (currentEvent.type == sf::Event::MouseButtonReleased)
	{
		if (m_isMovingEntity)
		{
			//std::pair<double, eDirection> mouseMoveDirection{ MouseSelection::calculateDirection(m_leftMouseDownPosition, sf::Mouse::getPosition(window)) };
			//switch (m_battle.getCurrentPhase())
			//{
			//case BattlePhase::Deployment:
			//{
			//	if (!m_tileOnClick.m_tile)
			//		break;
			//	if (mouseMoveDirection.first > 20)
			//	{
			//		onLeftClickDeploymentPhase(mouseMoveDirection.second);
			//		m_tileOnClick.m_tile = nullptr;
			//		m_mouseSprite.deactivate();
			//	}
			//	else
			//	{
			//		//This function call is to be used if the movement of the mouse during the move command is small enough to be considered unintended,
			//		//in this case the ship should not rotate after reaching the destination.
			//		onLeftClickDeploymentPhase();
			//		m_tileOnClick.m_tile = nullptr;
			//	}
			//	m_arrowActive = false;
			//	break;
			//}
			//case BattlePhase::Movement:
			//{
			//	if (!m_mouseDownTile || !m_tileOnClick.m_tile)
			//	{
			//		break;
			//	}
			//	if (mouseMoveDirection.first > 20)
			//	{
			//		m_battle.moveFactionShipToPosition(m_tileOnClick.m_tile->m_shipOnTile, m_mouseDownTile->m_tileCoordinate, mouseMoveDirection.second);
			//	}
			//	else
			//	{
			//		m_battle.moveFactionShipToPosition(m_tileOnClick.m_tile->m_shipOnTile, m_mouseDownTile->m_tileCoordinate);
			//	}
			//	m_arrowActive = false;
			//	m_movementArea.clear();
			//	break;
			//}
			//}
			////Resetting the variables used as triggers
			//m_mouseDownTile = nullptr;
			////TODO: Drop info box
			//m_tileOnClick.m_tile = nullptr;
			//m_isMovingEntity = false;
		}
	}
	else if (currentEvent.type == sf::Event::MouseMoved)
	{
		if (m_battle.getCurrentPlayerType() == ePlayerType::eAI)
		{
			return;
		}

		m_gui.onMouseMove(sf::Mouse::getPosition(window));
		handleMouseMovement(window);
	}
}

void BattleUI::update(float deltaTime)
{
	m_gui.update();
	//m_gui.update(m_battle.getMap().getWindDirection());// added update for gui to receive wind direction so compass direction updates
}

void BattleUI::FactionUpdateGUI(FactionName faction)
{
	//m_gui.updateFactionToken(faction);
}

void BattleUI::setCurrentFaction(FactionName faction)
{
	FactionUpdateGUI(faction);
}

bool BattleUI::isPaused()
{
	//return m_gui.isPaused();
	return false;
}

void BattleUI::clearTargetArea()
{
	m_targetArea.clearTargetArea();
}

void BattleUI::clearSelectedTile()
{
	m_arrowActive = false;
	if (m_tileOnClick && m_tileOnClick->m_shipOnTile.isValid())
	{
		m_battle.disableFactionShipMovementGraph(m_tileOnClick->m_shipOnTile);
	}
		
	m_tileOnClick = nullptr;
}

void BattleUI::renderTileHighlight(sf::RenderWindow& window)
{
	auto& map = m_battle.getMap();
	if (m_tileOnClick && (m_tileOnClick->m_type == eTileType::eSea || m_tileOnClick->m_type == eTileType::eOcean))
	{
		const sf::Vector2i tileTransform = map.getTileScreenPos(m_tileOnClick->m_tileCoordinate);

		m_tileHighlight.setPosition(sf::Vector2i(
			static_cast<float>(tileTransform.x + DRAW_OFFSET_X * map.getDrawScale()),
			static_cast<float>(tileTransform.y + DRAW_OFFSET_Y * map.getDrawScale())));

		m_tileHighlight.render(window, map);
	}
}

void BattleUI::onLeftClick(sf::RenderWindow & window)
{
	const Tile* tileOnMouse = m_battle.getMap().getTile(m_battle.getMap().getMouseClickCoord(m_lastMouseData));
	if (!tileOnMouse)
	{
		return;
	}

	if (!m_tileOnClick)
	{
		m_tileOnClick = tileOnMouse;
	}

	if (tileOnMouse->m_tileCoordinate != m_tileOnClick->m_tileCoordinate)
	{
		m_tileOnPreviousClick = m_tileOnClick;
		m_tileOnClick = tileOnMouse;
	}

	m_tileOnClick = tileOnMouse;
	m_tileOnMouse = tileOnMouse;

	switch (m_battle.getCurrentPhase())
	{
	case BattlePhase::Deployment:
	{
		m_leftMouseDownPosition = sf::Mouse::getPosition(window);
		onLeftClickDeploymentPhase();
		m_mouseSprite.deactivate();
		break;
	}
	case BattlePhase::Movement:
	{
		m_leftMouseDownPosition = sf::Mouse::getPosition(window);
		onLeftClickMovementPhase(m_lastMouseData);
		break;
	}
	case BattlePhase::Attack:
	{
		onLeftClickAttackPhase(m_lastMouseData);
		break;
	}
	}
}

void BattleUI::handleMouseMovement(sf::RenderWindow & window)
{
	//Ship selected doesn't match faction currently in play
	if (m_tileOnClick && m_tileOnClick->m_shipOnTile.isValid()
		&& m_battle.getFactionShip(m_tileOnClick->m_shipOnTile).getFactionName() != m_battle.getCurrentFaction())
	{
		return;
	}

	const Tile* tileOnMouse = m_battle.getMap().getTile(m_battle.getMap().getMouseClickCoord(m_lastMouseData));
	if (!tileOnMouse)
	{
		return;
	}

	if (!m_tileOnMouse)
	{
		m_tileOnMouse = tileOnMouse;
	}

	//New tile 
	if (m_tileOnMouse->m_tileCoordinate != tileOnMouse->m_tileCoordinate)
	{
		m_tileOnMouse = tileOnMouse;

		switch (m_battle.getCurrentPhase())
		{
		case BattlePhase::Deployment:
		{
			if (!m_isMovingEntity)
			{
				onMouseMoveDeploymentPhase(m_lastMouseData);
			}
			break;
		}
		case BattlePhase::Movement:
		{
			onMouseMoveMovementPhase(m_lastMouseData);
			break;
		}
		case BattlePhase::Attack:
		{
			onMouseMoveAttackPhase(m_lastMouseData);
			break;
		}
		}
	}
}

void BattleUI::onMouseMoveDeploymentPhase(sf::Vector2i mousePosition)
{
	//Only place ship on non occupied tile
	if (!m_tileOnMouse->m_shipOnTile.isValid())
	{
		m_battle.setShipDeploymentAtPosition(m_tileOnMouse->m_tileCoordinate);
	}

	m_mouseSprite.setPosition(m_tileOnMouse->m_tileCoordinate);
	m_mouseSprite.activate();
}

void BattleUI::onLeftClickDeploymentPhase(eDirection startingDirection)
{
	if ((m_tileOnClick->m_type == eTileType::eSea || m_tileOnClick->m_type == eTileType::eOcean))
	{
		//If tile isn't already occupied by a ship
		if (!m_tileOnClick->m_shipOnTile.isValid())
		{
			m_mouseSprite.setPosition(m_tileOnClick->m_tileCoordinate);
			m_battle.deployFactionShipAtPosition(m_tileOnClick->m_tileCoordinate, startingDirection);
		}
	}
}

void BattleUI::onMouseMoveMovementPhase(sf::Vector2i mousePosition)
{
	if (m_tileOnClick && m_tileOnClick->m_shipOnTile.isValid() &&  
		!m_battle.getFactionShip(m_tileOnClick->m_shipOnTile).isMovingToDestination() &&
		!m_battle.getFactionShip(m_tileOnClick->m_shipOnTile).isDestinationSet())
	{
		if (m_tileOnMouse->m_type == eTileType::eSea || m_tileOnMouse->m_type == eTileType::eOcean)
		{
			m_battle.generateFactionShipMovementGraph(m_tileOnClick->m_shipOnTile, m_tileOnMouse->m_tileCoordinate);
			m_mouseSprite.deactivate();
		}
		else
		{ 
			m_mouseSprite.setPosition(m_tileOnMouse->m_tileCoordinate, m_battle.getMap());
			m_mouseSprite.activate();
			m_battle.disableFactionShipMovementGraph(m_tileOnClick->m_shipOnTile);
		}	
	}
}

void BattleUI::onLeftClickMovementPhase(sf::Vector2i mousePosition)
{	
	//On first click
	if (!m_tileOnPreviousClick)
	{
		const Ship& ship = m_battle.getFactionShip(m_tileOnClick->m_shipOnTile);
		if (!ship.isDestinationSet())
		{
			m_movementArea.newArea(m_battle.getMap(), ship);
			return;
		}
	}

	//if (!m_tileOnClick && m_tileOnMouse->m_shipOnTile.isValid())
	//{
	//	m_tileOnClick = m_tileOnMouse;
	//	m_battle.disableFactionShipMovementGraph(m_tileOnClick->m_shipOnTile);
	//	//Display the available movement tiles
	//	const Ship& ship = m_battle.getFactionShip(m_tileOnClick->m_shipOnTile);
	//	if (!ship.isDestinationSet())
	//	{
	//		m_movementArea.newArea(m_battle.getMap(), ship);
	//	}
	//	
	//	return;
	//}

	if (!m_tileOnClick && m_tileOnMouse->m_shipOnTile.isValid() &&
		m_battle.getFactionShip(m_tileOnMouse->m_shipOnTile).getFactionName() != m_battle.getCurrentFaction())
	{
		m_tileOnClick = m_tileOnMouse;
		return;
	}

	//Invalid Location - Collidable tile
	if (m_tileOnMouse->m_type != eTileType::eSea && m_tileOnMouse->m_type != eTileType::eOcean)
	{
		if (m_tileOnClick && m_tileOnClick->m_shipOnTile.isValid())
		{
			m_battle.disableFactionShipMovementGraph(m_tileOnClick->m_shipOnTile); 
		}
		return;
	}

	//Do not select killed entity
	if (m_tileOnMouse->m_shipOnTile.isValid() && m_battle.getFactionShip(m_tileOnMouse->m_shipOnTile).isDead())
	{
		//TODO: Drop info box
		m_tileOnClick = nullptr;
		m_movementArea.clear();
		return;
	}

	//Clicking to where entity is moving to
	if (m_tileOnMouse->m_shipOnTile.isValid() && m_battle.getFactionShip(m_tileOnMouse->m_shipOnTile).isDestinationSet())
	{
		//TODO: Drop info box
		m_tileOnClick = nullptr;
		m_movementArea.clear();
		return;
	}

	if (m_tileOnClick)
	{
		//Cancel movement if clicked on same entity
		if (m_tileOnClick->m_tileCoordinate == m_tileOnMouse->m_tileCoordinate)
		{
			m_battle.disableFactionShipMovementGraph(m_tileOnClick->m_shipOnTile);
			//TODO: Drop info box
			m_tileOnClick = nullptr;
		}

		//Disallow movement to tile occupied by other player
		else if (m_tileOnMouse->m_shipOnTile.isValid())
		{
			m_battle.disableFactionShipMovementGraph(m_tileOnClick->m_shipOnTile); 
			m_tileOnClick = m_tileOnMouse;

			const Ship& ship = m_battle.getFactionShip(m_tileOnClick->m_shipOnTile);
			if (ship.getFactionName() == m_battle.getCurrentFaction() && !ship.isDestinationSet())
			{
				m_movementArea.newArea(m_battle.getMap(), ship);
			}
		}

		//Store data so Entity can move to new location
		else if (m_tileOnClick->m_shipOnTile.isValid() && (m_tileOnClick->m_tileCoordinate != m_tileOnMouse->m_tileCoordinate)
			&& m_battle.getFactionShip(m_tileOnClick->m_shipOnTile).getFactionName() == m_battle.getCurrentFaction())
		{
			assert(m_battle.getFactionShip(m_tileOnClick->m_shipOnTile).getFactionName() == m_battle.getCurrentFaction());
			m_mouseDownTile = m_tileOnMouse;
			m_isMovingEntity = true;
			auto test = m_battle.getFactionShip(m_tileOnClick->m_shipOnTile).getEndOfMovementPath();
			if (m_battle.getFactionShip(m_tileOnClick->m_shipOnTile).getEndOfMovementPath() == m_tileOnMouse->m_tileCoordinate)
				m_arrowActive = true;

			m_battle.moveFactionShipToPosition(m_tileOnClick->m_shipOnTile, m_tileOnMouse->m_tileCoordinate);
			m_tileOnClick = nullptr;
		}
	}
	else
	{
		if (m_tileOnMouse->m_shipOnTile.isValid() && m_battle.getFactionShip(m_tileOnMouse->m_shipOnTile).isDead())
		{
			//TODO: Drop info box
			m_tileOnClick = nullptr;
			m_movementArea.clear();
		}
		//Do not select tile that contains wrong players entity
		if (m_tileOnMouse->m_shipOnTile.isValid())
		{
			if (m_battle.getFactionShip(m_tileOnMouse->m_shipOnTile).getFactionName() != m_battle.getCurrentFaction())
			{
				//TODO: Drop info box
				m_tileOnClick = nullptr;
				m_movementArea.clear();
			}
			//Selecting another ship you own
			else
			{
				m_tileOnClick = m_tileOnMouse;
			}
		}
	}
}

void BattleUI::onRightClickMovementPhase(sf::Vector2i mousePosition)
{
	assert(m_battle.getCurrentPhase() == BattlePhase::Movement);
	//Cancel selected Entity
	if (m_tileOnClick && m_tileOnClick->m_shipOnTile.isValid())
	{
		m_battle.disableFactionShipMovementGraph(m_tileOnClick->m_shipOnTile); 
		m_mouseSprite.deactivate();
	}
	//TODO: Drop info box
	m_tileOnClick = nullptr;
	m_movementArea.clear();
}

void BattleUI::onLeftClickAttackPhase(sf::Vector2i mousePosition)
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
	const Tile* tileOnMouse = m_battle.getMap().getTile(m_battle.getMap().getMouseClickCoord(mousePosition));
	if (!tileOnMouse)
	{
		return;
	}

	if (m_battle.getCurrentPlayerType() == ePlayerType::eAI)
	{
		m_tileOnClick = tileOnMouse;
		return;
	}

	//Select new entity that is on same team
	if (m_tileOnClick && m_tileOnClick->m_shipOnTile.isValid() && tileOnMouse->m_shipOnTile.isValid() &&
		(m_battle.getFactionShip(tileOnMouse->m_shipOnTile).getFactionName() == m_battle.getCurrentFaction()))
	{
		if (!m_battle.getFactionShip(tileOnMouse->m_shipOnTile).isWeaponFired())
		{
			m_targetArea.clearTargetArea();
			m_targetArea.generateTargetArea(m_battle, *tileOnMouse);
			m_tileOnClick = tileOnMouse;
			return;
		}
	}

	if (!m_tileOnClick && tileOnMouse->m_shipOnTile.isValid() && m_battle.getFactionShip(tileOnMouse->m_shipOnTile).getFactionName() != m_battle.getCurrentFaction())
	{
		m_tileOnClick = tileOnMouse;
		return;
	}

	//Do not fire on destroyed ship
	if (tileOnMouse->m_shipOnTile.isValid() && m_battle.getFactionShip(tileOnMouse->m_shipOnTile).isDead())
	{
		m_targetArea.clearTargetArea();
		m_mouseSprite.deactivate();
		//TODO: Drop info box
		m_tileOnClick = nullptr;
		return;
	}

	//Clicking on the same entity that has been previously selected
	if (m_tileOnClick && m_tileOnClick->m_tileCoordinate == tileOnMouse->m_tileCoordinate)
	{
		m_targetArea.clearTargetArea();
		m_mouseSprite.deactivate();
		//TODO: Drop info box
		m_tileOnClick = nullptr;
		return;
	}

	//Entity already selected Fire weapon at position
	//If the selectedTile has an entity on it and that entity hasn't fired
	if (m_tileOnClick && m_tileOnClick->m_shipOnTile.isValid() && 
		!m_battle.getFactionShip(m_tileOnClick->m_shipOnTile).isWeaponFired())
	{
		//If there is an entity on the tile you're clicking on and that entity's faction name differs from the one of the ship that's firing
		if ((tileOnMouse->m_shipOnTile.isValid()) && m_battle.getFactionShip(tileOnMouse->m_shipOnTile).getFactionName() != 
			m_battle.getFactionShip(m_tileOnClick->m_shipOnTile).getFactionName())
		{
			m_battle.fireFactionShipAtPosition(m_tileOnClick->m_shipOnTile, tileOnMouse->m_shipOnTile, m_targetArea.m_targetArea);
		}
		m_targetArea.clearTargetArea();
		m_tileOnClick = nullptr;
		m_mouseSprite.deactivate();
		return;
	}

	//Entity Already Selected whilst showing where to fire
	//Change to different Entity before firing
	if (tileOnMouse->m_shipOnTile.isValid() && m_battle.getFactionShip(tileOnMouse->m_shipOnTile).getFactionName() == m_battle.getCurrentFaction()
		 && m_tileOnClick && m_targetArea.m_targetArea.size() > 0)
	{
		m_targetArea.clearTargetArea();
		m_targetArea.generateTargetArea(m_battle, *tileOnMouse);
		//TODO: Raise info box
		m_tileOnClick = tileOnMouse;
		return;
	}

	//Click on same
	//Select new Entity to fire at something
	if (tileOnMouse->m_shipOnTile.isValid() && m_battle.getFactionShip(tileOnMouse->m_shipOnTile).getFactionName() != m_battle.getCurrentFaction())
	{
		m_tileOnClick = tileOnMouse;
		return;
	}

	if (!m_tileOnClick && tileOnMouse->m_shipOnTile.isValid() && m_battle.getFactionShip(tileOnMouse->m_shipOnTile).getFactionName() == 
		m_battle.getCurrentFaction())
	{
		m_tileOnClick = tileOnMouse;
	}

	if (m_tileOnClick && m_tileOnClick->m_shipOnTile.isValid() && 
		!m_battle.getFactionShip(m_tileOnClick->m_shipOnTile).isWeaponFired() &&
		m_battle.getFactionShip(m_tileOnClick->m_shipOnTile).getFactionName() == m_battle.getCurrentFaction())
	{
		m_tileOnClick = tileOnMouse;
		m_targetArea.generateTargetArea(m_battle, *tileOnMouse);
	}
}

void BattleUI::onRightClickAttackPhase(sf::Vector2i mousePosition)
{
	//TODO: Drop info box
	m_tileOnClick = nullptr;
	m_mouseSprite.deactivate();
	m_targetArea.clearTargetArea();
}

void BattleUI::onMouseMoveAttackPhase(sf::Vector2i mousePosition)
{
	auto tileCoordinate = m_tileOnMouse->m_tileCoordinate;
	//tileOnMouse in new position
	if (m_tileOnClick && m_tileOnClick->m_tileCoordinate != m_tileOnMouse->m_tileCoordinate)
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
			m_mouseSprite.setPosition(m_tileOnMouse->m_tileCoordinate, m_battle.getMap());
			m_mouseSprite.activate();
		}
		//outside weapon range
		else
		{
			m_mouseSprite.deactivate();
		}
	}
}

void BattleUI::onResetBattle()
{
	m_targetArea.onReset();
	m_tileOnClick = nullptr;
	m_leftMouseDownPosition = sf::Vector2i(0, 0);
	m_mouseDownTile = nullptr;
	m_arrowActive = false;
	m_lastMouseData = { 0,0 };
}

void BattleUI::onNewTurn()
{
	m_tileOnClick = nullptr;
	m_mouseSprite.deactivate();
	clearTargetArea();
	clearSelectedTile();
	m_movementArea.clear();
}

//Weapon Graph
BattleUI::TargetArea::TargetArea()
{
	for (auto& i : m_targetAreaGraph)
	{
		i.setTexture(Textures::getInstance().m_cross);
	}
}

void BattleUI::TargetArea::render(sf::RenderWindow& window, const Map& map)
{
	for (auto& i : m_targetAreaGraph)
	{
		if (i.isActive())
		{
			i.render(window, map);
			//const sf::Vector2i tileTransform = map.getTileScreenPos(i.position);

			//i.sprite.setPosition({
			//static_cast<float>(tileTransform.x + DRAW_OFFSET_X * map.getDrawScale()),
			//static_cast<float>(tileTransform.y + DRAW_OFFSET_Y * map.getDrawScale()) });
			//window.draw(i.sprite);
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
	assert(!m_targetAreaGraph.empty());
	//using same convention as movement // from source should be able to get position
	for (int i = 0; i < m_targetArea.size(); i++)
	{
		if (!m_targetArea[i])//Check that i is not nullptr
			continue;

		sf::Vector2i tilePos = battle.getMap().getTileScreenPos(m_targetArea[i]->m_tileCoordinate);
		m_targetAreaGraph[i].setPosition(sf::Vector2i(
				 tilePos.x + 12 * battle.getMap().getDrawScale(),
				 tilePos.y + 28 * battle.getMap().getDrawScale()
			));

		m_targetAreaGraph[i].activate();
		m_targetAreaGraph[i].setPosition(m_targetArea[i]->m_tileCoordinate);
	}
}

void BattleUI::TargetArea::clearTargetArea()
{
	for (auto& i : m_targetAreaGraph)
	{
		if (!i.isActive())
		{
			break;
		}

		i.deactivate();
	}
}

void BattleUI::TargetArea::onReset()
{
	//m_targetAreaSprites.clear();
	m_targetArea.clear();
}

BattleUI::MovementArea::MovementArea(std::unique_ptr<Texture>& texturePtr, const Map& map)
	: m_display(false),
	m_displaySize(0),
	m_movementArea()
{
	float scale = map.getDrawScale();
	for (auto& i : m_movementArea)
	{
		i.second.setTexture(texturePtr);
		i.second.setScale(sf::Vector2f(scale, scale));
	}
}

void BattleUI::MovementArea::render(sf::RenderWindow& window, const Map& map)
{
	if (m_display)
	{
		float scale = map.getDrawScale();
		for (int i = 0; i < m_displaySize; i++)
		{
			posi pos = map.getTileScreenPos(m_movementArea[i].first);
			float x = static_cast<float>(pos.x) + DRAW_OFFSET_X * scale;
			float y = static_cast<float>(pos.y) + DRAW_OFFSET_Y * scale;
			m_movementArea[i].second.setPosition(sf::Vector2i( x, y ));
			m_movementArea[i].second.render(window);
		}
	}
}

void BattleUI::MovementArea::newArea(const Map& map, const Ship& ship)
{
	posi startPos = { ship.getCurrentPosition(), ship.getCurrentDirection() };
	std::vector<posi> area = BFS::findArea(map, startPos, ship.getMovementPoints());
	
	m_displaySize = static_cast<size_t>(area.size());
	for (int i = 0; i < m_displaySize; i++)
	{
		m_movementArea[i].first = area[i].pair();
	}

	m_display = true;
}

void BattleUI::MovementArea::clear()
{
	m_display = false;
	m_displaySize = 0;
}