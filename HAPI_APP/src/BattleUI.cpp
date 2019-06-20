#include "BattleUI.h"
#include "Battle.h"
#include "BFS.h"
#include "Textures.h"
#include "Utilities/Utilities.h"
#include "GameEventMessenger.h"
#include <assert.h>
#include "AI.h"

//Debug
#include <iostream>

constexpr size_t MAX_MOVE_AREA{ 700 };
constexpr size_t MAX_TARGET_AREA = 50;

BattleUI::BattleUI(Battle & battle)
	: m_battle(battle),
	m_tileOnPreviousLeftClick(nullptr),
	m_tileOnLeftClick(nullptr),
	m_tileOnMouse(nullptr),
	m_spriteOnTileClick(),
	m_spriteOnMouse(Textures::getInstance().m_cross, false),
	m_movementArea(Textures::getInstance().m_selectedHex, MAX_MOVE_AREA, m_battle.getMap()),
	m_targetArea(Textures::getInstance().m_mouseCrossHair, MAX_TARGET_AREA, m_battle.getMap()),
	m_leftClickHeld(false),
	m_leftClickPosition(),
	m_maxCameraOffset(),
	m_pendingCameraMovement(),
	m_cameraPositionOffset()
{
	GameEventMessenger::getInstance().subscribe(std::bind(&BattleUI::onNewBattlePhase, this, std::placeholders::_1), eGameEvent::eEnteredNewBattlePhase);
}

BattleUI::~BattleUI()
{
	GameEventMessenger::getInstance().unsubscribe(eGameEvent::eEnteredNewBattlePhase);
}

sf::Vector2i BattleUI::getCameraPositionOffset() const
{
	return m_cameraPositionOffset;
}

void BattleUI::render(sf::RenderWindow& window)
{
	m_spriteOnTileClick.render(window, m_battle.getMap());
	m_movementArea.render(window, m_battle.getMap());
	m_battle.renderFactionShipsMovementGraphs(window);

	for (auto& i : m_targetArea.m_tileAreaGraph)
	{
		i.render(window, m_battle.getMap());
	}

	if (m_tileOnLeftClick && (m_tileOnLeftClick->m_type == eTileType::eSea || m_tileOnLeftClick->m_type == eTileType::eOcean))
	{
		m_spriteOnTileClick.render(window, m_battle.getMap());
	}

	m_spriteOnMouse.render(window, m_battle.getMap());

	m_shipSelector.renderShipHighlight(window, m_battle.getMap());
	if (m_leftClickHeld)
	{
		m_shipSelector.renderSelector(window);
	}
}

void BattleUI::setMaxCameraOffset(sf::Vector2i maxCameraOffset)
{
	m_maxCameraOffset = sf::Vector2i(maxCameraOffset.x * 24 - 820, maxCameraOffset.y * 28 - 400);
	if (m_maxCameraOffset.x < 0)
	{
		m_maxCameraOffset.x = 0;
	}
	if (m_maxCameraOffset.y < 0)
	{
		m_maxCameraOffset.y = 0;
	}
}

void BattleUI::handleInput(const sf::RenderWindow& window, const sf::Event & currentEvent)
{
	sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
	ePlayerType currentPlayerType = m_battle.getCurrentPlayerType();

	switch (currentEvent.type)
	{
	case sf::Event::MouseButtonPressed:
		if (currentEvent.mouseButton.button == sf::Mouse::Left)
		{
			onLeftClick(mousePosition);
		}
		else if (currentEvent.mouseButton.button == sf::Mouse::Right && currentPlayerType == ePlayerType::eHuman)
		{
			onRightClick(mousePosition);
		}
		break;

	case sf::Event::MouseMoved:
		onMouseMove(mousePosition);
		break;

	case sf::Event::KeyPressed:
		if (currentPlayerType == ePlayerType::eHuman)
		{
			GameEventMessenger::getInstance().broadcast(GameEvent(), eGameEvent::eEndBattlePhaseEarly);
		}
		break;

	case sf::Event::MouseButtonReleased:
		if (currentPlayerType == ePlayerType::eHuman)
		{
			onClickReleased(mousePosition);
		}
		break;
	}
}

void BattleUI::update(float deltaTime)
{
	updateCamera();
}

void BattleUI::generateTargetArea(const Tile & source)
{
	m_targetArea.clearTileArea();

	const Ship& ship = m_battle.getFactionShip(source.m_shipOnTile);
	if (ship.getShipType() == eShipType::eFrigate)
	{
		m_battle.getMap().getTileCone(m_targetArea.m_tileArea, source.m_tileCoordinate,
			ship.getRange(),
			ship.getCurrentDirection(), true);

	}
	else if (ship.getShipType() == eShipType::eSniper)
	{
		m_battle.getMap().getTileLine(m_targetArea.m_tileArea, source.m_tileCoordinate,
			ship.getRange(),
			ship.getCurrentDirection(), true);

	}
	else if (ship.getShipType() == eShipType::eTurtle)
	{
		// make so where ever the place presses get radius called talk adrais about size of that
		m_battle.getMap().getTileRadius(m_targetArea.m_tileArea, source.m_tileCoordinate,
			ship.getRange(), true);
	}
	else if (ship.getShipType() == eShipType::eFire)
	{
		eDirection directionOfFire = eNorth;
		switch (ship.getCurrentDirection())
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

		m_battle.getMap().getTileLine(m_targetArea.m_tileArea, source.m_tileCoordinate,
			ship.getRange(), directionOfFire, true);	
	}

	m_targetArea.activateGraph();
}

void BattleUI::generateMovementArea(const Ship & ship)
{
	m_movementArea.clearTileArea();

	posi startPos = { ship.getCurrentPosition(), ship.getCurrentDirection() };
	BFS::findArea(m_movementArea.m_tileArea, m_battle.getMap(), startPos, ship.getMovementPoints());

	m_movementArea.activateGraph();
}

void BattleUI::updateCamera()
{
	//camera pan
	if (m_pendingCameraMovement != sf::Vector2f(0, 0))
	{
		m_cameraPositionOffset.x += m_pendingCameraMovement.x;//translates the camera position
		m_cameraPositionOffset.y += m_pendingCameraMovement.y;

		if (m_cameraPositionOffset.x < -120)//checks for if its reached any of the 4 boundries, need to change it to a width variable
		{
			m_cameraPositionOffset.x = -120;
		}
		else if (m_cameraPositionOffset.x > m_maxCameraOffset.x)
		{
			m_cameraPositionOffset.x = m_maxCameraOffset.x;
		}
		else
		{
			m_cameraPositionOffset.x += m_pendingCameraMovement.x;
		}

		if (m_cameraPositionOffset.y < -100)
		{
			m_cameraPositionOffset.y = -100;
		}
		else if (m_cameraPositionOffset.y > m_maxCameraOffset.y)
		{
			m_cameraPositionOffset.y = m_maxCameraOffset.y;
		}
		else
		{
			m_cameraPositionOffset.y += m_pendingCameraMovement.y;
		}
	}
}

void BattleUI::onClickReleased(sf::Vector2i mousePosition)
{
	if (!m_leftClickHeld)
	{
		return;
	}

	m_leftClickHeld = false;

	auto mouseDirection = Math::calculateDirection(m_leftClickPosition, mousePosition);	
	if (m_battle.getCurrentBattlePhase() == BattlePhase::Deployment)
	{
		onLeftClickDeploymentPhase(mouseDirection.second);	
	}
	else if (m_battle.getCurrentBattlePhase() == BattlePhase::Movement)
	{
		onLeftClickMovementPhase(mouseDirection, mousePosition);
	}
}

void BattleUI::onLeftClick(sf::Vector2i mousePosition)
{
	if (m_battle.getCurrentBattlePhase() == BattlePhase::Deployment || 
		m_battle.getCurrentBattlePhase() == BattlePhase::Movement)
	{
		m_leftClickHeld = true;
	}

	m_leftClickPosition = mousePosition;
	
	m_shipSelector.resetShape();
	m_shipSelector.setPosition(mousePosition);
	
	if (m_shipSelector.isShipsSelected())
	{
		std::vector<const Tile*> adjacentTiles = m_battle.getMap().getAdjacentTiles(m_tileOnLeftClick->m_tileCoordinate);
		for (auto& tile : adjacentTiles)
		{
			if (tile && !tile->isShipOnTile())
			{
				ShipOnTile selectedShip = m_shipSelector.getSelectedShip();
				m_battle.moveFactionShipToPosition(selectedShip, tile->m_tileCoordinate);

				if (!m_shipSelector.isShipsSelected())
				{
					break;
				}
			}		
		}
	}

	const Tile* tileOnMouse = m_battle.getMap().getTile(m_battle.getMap().getMouseClickCoord(mousePosition));
	if (!tileOnMouse)
	{
		return;
	}

	//First Click
	if (!m_tileOnLeftClick)
	{
		m_tileOnLeftClick = tileOnMouse;
		m_tileOnMouse = tileOnMouse;
	}

	//Clicked on new tile
	if (m_tileOnLeftClick->m_tileCoordinate != tileOnMouse->m_tileCoordinate)
	{
		m_tileOnPreviousLeftClick = m_tileOnLeftClick;
		m_tileOnLeftClick = tileOnMouse;
		m_tileOnMouse = tileOnMouse;
	}

	//Clicked on tile containing non usable ship
	if (m_tileOnLeftClick->isShipOnTile())
	{
		auto& ship = m_battle.getFactionShip(m_tileOnLeftClick->m_shipOnTile);
		if (ship.isDead())
		{
			m_tileOnPreviousLeftClick = nullptr;
			return;
		}
	}

	if (m_battle.getCurrentBattlePhase() == BattlePhase::Attack)
	{
		onLeftClickAttackPhase(mousePosition);
	}
}

void BattleUI::onLeftClickAttackPhase(sf::Vector2i mousePosition)
{
	if (!m_tileOnPreviousLeftClick && m_tileOnLeftClick->isShipOnTile() && !m_battle.getFactionShip(m_tileOnLeftClick->m_shipOnTile).isWeaponFired()
		&& m_battle.isShipBelongToCurrentFactionInPlay(m_tileOnLeftClick->m_shipOnTile))
	{
		generateTargetArea(*m_tileOnLeftClick);
		return;
	}

	if (m_tileOnLeftClick->isShipOnTile() && m_tileOnPreviousLeftClick && 
		!m_battle.isShipBelongToCurrentFactionInPlay(m_tileOnLeftClick->m_shipOnTile))
	{
		m_battle.fireFactionShipAtPosition(m_tileOnPreviousLeftClick->m_shipOnTile, *m_tileOnLeftClick, m_targetArea.m_tileArea);
	}
	else if (!m_tileOnLeftClick->isShipOnTile() && m_tileOnPreviousLeftClick)
	{
		m_battle.fireFactionShipAtPosition(m_tileOnPreviousLeftClick->m_shipOnTile, *m_tileOnLeftClick, m_targetArea.m_tileArea);
	}

	m_targetArea.clearTileArea();
	m_tileOnLeftClick = nullptr;
	m_tileOnPreviousLeftClick = nullptr;
	m_tileOnMouse = nullptr;
	m_spriteOnTileClick.deactivate();
	m_spriteOnMouse.deactivate();
}

void BattleUI::onMouseMove(sf::Vector2i mousePosition)
{
	moveCamera(mousePosition);

	if (m_leftClickHeld)
	{
		m_shipSelector.update(m_battle.getCurrentFactionShips(), mousePosition, m_battle.getMap());
	}

	//Ship selected doesn't match faction currently in play
	if ((m_tileOnLeftClick && m_tileOnLeftClick->isShipOnTile()
		&& !m_battle.isShipBelongToCurrentFactionInPlay(m_tileOnLeftClick->m_shipOnTile))
		|| m_battle.getCurrentPlayerType() == ePlayerType::eAI)
	{
		return;
	}

	const Tile* tileOnMouse = m_battle.getMap().getTile(m_battle.getMap().getMouseClickCoord(mousePosition));
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

		if (m_shipSelector.isShipsSelected())
		{
			std::vector<const Tile*> adjacentTiles = m_battle.getMap().getAdjacentTiles(m_tileOnMouse->m_tileCoordinate);
			for (const auto& tile : adjacentTiles)
			{
				if (tile && !tile->isShipOnTile())
				{
					for (auto selectedShip : m_shipSelector.getSelectedShips())
					{
						if (!selectedShip.m_shipOnTile.isValid())
						{
							break;
						}

						m_battle.generateFactionShipMovementGraph(selectedShip.m_shipOnTile, tile->m_tileCoordinate);
					}
				}
			}
		}

		switch (m_battle.getCurrentBattlePhase())
		{
		case BattlePhase::Deployment:
		{
			onMouseMoveDeploymentPhase(mousePosition);
			break;
		}
		case BattlePhase::Movement:
		{
			//onMouseMoveMovementPhase(mousePosition);
			break;
		}
		case BattlePhase::Attack:
		{
			onMouseMoveAttackPhase(mousePosition);
			break;
		}
		}
	}
}

void BattleUI::moveCamera(sf::Vector2i mousePosition)
{
	m_pendingCameraMovement = sf::Vector2f(0, 0);

	if (mousePosition.x < 100)
	{
		m_pendingCameraMovement += sf::Vector2f{ -1,0 };
	}
	else if (mousePosition.x > 1820)
	{
		m_pendingCameraMovement += sf::Vector2f{ 1,0 };
	}

	if (mousePosition.y < 50)
	{
		m_pendingCameraMovement += sf::Vector2f{ 0 , -1 };
	}
	else if (mousePosition.y > 980)
	{
		m_pendingCameraMovement += sf::Vector2f{ 0, 1 };
	}
}

void BattleUI::onMouseMoveDeploymentPhase(sf::Vector2i mousePosition)
{
	//Only place ship on non occupied tile
	if (!m_tileOnMouse->isShipOnTile())
	{
		m_battle.setShipDeploymentAtPosition(m_tileOnMouse->m_tileCoordinate);
	}
}

void BattleUI::onLeftClickDeploymentPhase(eDirection startingDirection)
{
	if ((m_tileOnLeftClick->m_type == eTileType::eSea || m_tileOnLeftClick->m_type == eTileType::eOcean))
	{
		//If tile isn't already occupied by a ship
		if (!m_tileOnLeftClick->isShipOnTile())
		{
			m_spriteOnTileClick.setPosition(m_tileOnLeftClick->m_tileCoordinate);
			m_battle.deployFactionShipAtPosition(m_tileOnLeftClick->m_tileCoordinate, startingDirection);
		}
	}
}

void BattleUI::onMouseMoveMovementPhase(sf::Vector2i mousePosition)
{
	if (m_tileOnLeftClick && m_tileOnLeftClick->isShipOnTile() &&
		(m_tileOnMouse->m_type == eTileType::eSea || m_tileOnMouse->m_type == eTileType::eOcean))
	{
		m_battle.generateFactionShipMovementArea(m_tileOnClick->m_shipOnTile, m_tileOnMouse->m_tileCoordinate);
	}
	else if(m_tileOnLeftClick && m_tileOnLeftClick->isShipOnTile())
	{
		m_battle.clearFactionShipMovementArea(m_tileOnClick->m_shipOnTile);
	}
}

void BattleUI::onLeftClickMovementPhase(std::pair<double, eDirection> mouseDirection, sf::Vector2i mousePosition)
{	
	//On first Ship Selection
	if (!m_tileOnPreviousLeftClick && m_tileOnLeftClick->isShipOnTile() && 
		m_battle.isShipBelongToCurrentFactionInPlay(m_tileOnLeftClick->m_shipOnTile))
	{
		const Ship& ship = m_battle.getFactionShip(m_tileOnLeftClick->m_shipOnTile);
		if (!ship.isDestinationSet())
		{
			generateMovementArea(ship);
		}
	}
	//Ship already selected
	else if(m_tileOnPreviousLeftClick && m_tileOnPreviousLeftClick->isShipOnTile())
	{	
		if (!m_tileOnLeftClick->isShipOnTile() && m_movementArea.isPositionInTileArea(m_tileOnLeftClick->m_tileCoordinate))
		{
			if (Math::isFacingDifferentTile(mouseDirection.first))
			{
				m_battle.moveFactionShipToPosition(m_tileOnPreviousClick->m_shipOnTile, mouseDirection.second);
			}
			else
			{
				m_battle.moveFactionShipToPosition(m_tileOnPreviousClick->m_shipOnTile);
			}
		}
		else if (!m_tileOnLeftClick->isShipOnTile() && !m_movementArea.isPositionInTileArea(m_tileOnLeftClick->m_tileCoordinate))
		{
			m_battle.clearFactionShipMovementArea(m_tileOnPreviousClick->m_shipOnTile);
		}
		else if (m_tileOnLeftClick->isShipOnTile())
		{
			m_battle.clearFactionShipMovementArea(m_tileOnPreviousClick->m_shipOnTile);
		}

		m_tileOnLeftClick = nullptr;
		m_tileOnPreviousLeftClick = nullptr;
		m_movementArea.clearTileArea();
	}
}

void BattleUI::onRightClick(sf::Vector2i mousePosition)
{
	m_leftClickHeld = false;
	

	switch (m_battle.getCurrentBattlePhase())
	{
	case BattlePhase::Movement:
	{
		onRightClickMovementPhase(mousePosition);
		break;
	}
	case BattlePhase::Attack:
	{
		onRightClickAttackPhase(mousePosition);
		break;
	}
	}

	m_shipSelector.reset();
}

void BattleUI::onRightClickMovementPhase(sf::Vector2i mousePosition)
{
	//Cancel selected ships within box
	if (m_shipSelector.isShipsSelected())
	{
		for (auto& selectedShip : m_shipSelector.getSelectedShips())
		{
			if (selectedShip.m_shipOnTile.isValid())
			{
				m_battle.disableFactionShipMovementGraph(selectedShip.m_shipOnTile);
			}
		}
	}
	//Cancel individually selected Ship
	else if (m_tileOnLeftClick && m_tileOnLeftClick->isShipOnTile())
	{
		m_battle.disableFactionShipMovementGraph(m_tileOnLeftClick->m_shipOnTile); 
		m_spriteOnTileClick.deactivate();
	}

	m_tileOnLeftClick = nullptr;
	m_tileOnPreviousLeftClick = nullptr;
	m_movementArea.clearTileArea();
}

void BattleUI::onRightClickAttackPhase(sf::Vector2i mousePosition)
{
	m_tileOnLeftClick = nullptr;
	m_spriteOnTileClick.deactivate();
	m_targetArea.clearTileArea();
}

void BattleUI::onMouseMoveAttackPhase(sf::Vector2i mousePosition)
{
	auto tileCoordinate = m_tileOnMouse->m_tileCoordinate;
	auto cIter = std::find_if(m_targetArea.m_tileArea.cbegin(), m_targetArea.m_tileArea.cend(),
		[tileCoordinate](const auto& tile) { return tileCoordinate == tile->m_tileCoordinate; });
	
	if (cIter != m_targetArea.m_tileArea.cend())
	{
		m_spriteOnMouse.setPosition(m_tileOnMouse->m_tileCoordinate);
		m_spriteOnMouse.activate();
	}
	else
	{
		m_spriteOnMouse.deactivate();
	}
}

void BattleUI::onNewBattlePhase(GameEvent gameEvent)
{
	m_tileOnLeftClick = nullptr;
	m_tileOnPreviousLeftClick = nullptr;
	m_tileOnMouse = nullptr;
	m_spriteOnTileClick.deactivate();
	m_targetArea.clearTileArea();
	m_movementArea.clearTileArea();
}