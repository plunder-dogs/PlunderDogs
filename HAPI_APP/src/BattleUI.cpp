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
	m_tileOnLeftClick(nullptr),
	m_tileOnMouse(nullptr),
	m_spriteOnTileClick(),
	m_spriteOnMouse(*Textures::getInstance().m_cross, false),
	m_movementArea(*Textures::getInstance().m_selectedHex, MAX_MOVE_AREA),
	m_targetArea(*Textures::getInstance().m_mouseCrossHair, MAX_TARGET_AREA),
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
	m_targetArea.render(window, m_battle.getMap());

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
	mousePosition += MOUSE_POSITION_OFFSET;

	switch (currentEvent.type)
	{
	case sf::Event::MouseButtonPressed:
		if (currentEvent.mouseButton.button == sf::Mouse::Left)
		{
			onLeftClick(mousePosition);
		}
		else if (currentEvent.mouseButton.button == sf::Mouse::Right)
		{
			onRightClick(mousePosition);
		}
		break;

	case sf::Event::MouseMoved:
		moveCamera(mousePosition);
		onMouseMove(mousePosition);
		break;

	case sf::Event::KeyPressed:
		onKeyPress(mousePosition, currentEvent);
		break;

	case sf::Event::MouseButtonReleased:
		if (m_battle.getCurrentPlayerType() == ePlayerType::eHuman)
		{
			if (m_leftClickHeld)
			{
				onLeftClickReleased(mousePosition);
			}
			else if(m_rightClickHeld)
			{
				onRightClickReleased(mousePosition);
			}
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
	switch (ship.getShipType())
	{
	case eShipType::eFrigate: 
		m_battle.getMap().getTileCone(m_targetArea.m_tileArea, source.m_tileCoordinate,
			ship.getRange(),
			ship.getCurrentDirection(), true);
		break;

	case eShipType::eSniper :
		m_battle.getMap().getTileLine(m_targetArea.m_tileArea, source.m_tileCoordinate,
			ship.getRange(),
			ship.getCurrentDirection(), true);
		break;

	case eShipType::eTurtle :
		m_battle.getMap().getTileRadius(m_targetArea.m_tileArea, source.m_tileCoordinate,
			ship.getRange(), true);
		break;

	case eShipType::eFire :
		m_battle.getMap().getTileLine(m_targetArea.m_tileArea, source.m_tileCoordinate,
			ship.getRange(), Utilities::getOppositeDirection(ship.getCurrentDirection()), true);
		break;
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

void BattleUI::onKeyPress(sf::Vector2i mousePosition, const sf::Event& currentEvent)
{
	if (currentEvent.key.code == sf::Keyboard::Escape)
	{
		m_leftClickHeld = false;
		m_rightClickHeld = false;

		switch (m_battle.getCurrentBattlePhase())
		{
		case BattlePhase::Movement:
		{
			onCancelMovementPhase(mousePosition);
			break;
		}
		case BattlePhase::Attack:
		{
			onCancelAttackPhase(mousePosition);
			break;
		}
		}

		m_shipSelector.reset();
	}
	else if (currentEvent.key.code == sf::Keyboard::Enter && m_battle.getCurrentPlayerType() == ePlayerType::eHuman)
	{
		if (m_battle.getCurrentPlayerType() == ePlayerType::eHuman)
		{
			GameEventMessenger::getInstance().broadcast(GameEvent(), eGameEvent::eEndBattlePhaseEarly);
		}
	}
}

void BattleUI::updateCamera()
{
	//camera pan
	if (m_pendingCameraMovement != sf::Vector2f(0, 0))
	{
		m_cameraPositionOffset.x += m_pendingCameraMovement.x;
		m_cameraPositionOffset.y += m_pendingCameraMovement.y;

		if (m_cameraPositionOffset.x < -120)
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

void BattleUI::onLeftClickReleased(sf::Vector2i mousePosition)
{
	m_leftClickHeld = false;
	
	if (!m_battle.isShipBelongToFactionInPlay(m_tileOnLeftClick->m_shipOnTile))
	{
		return;
	}

	if (m_battle.getCurrentBattlePhase() == BattlePhase::Movement)
	{
		onLeftClickMovementPhase(mousePosition);
	}
	else if (m_battle.getCurrentBattlePhase() == BattlePhase::Attack)
	{
		onLeftClickAttackPhase(mousePosition);
	}
}

void BattleUI::onRightClickReleased(sf::Vector2i mousePosition)
{
	m_rightClickHeld = false;

	auto mouseDirection = Math::calculateDirection(m_rightClickPosition, mousePosition);
	switch (m_battle.getCurrentBattlePhase())
	{
	case BattlePhase::Deployment:
		onRightClickDeploymentPhase(mouseDirection.second);
		break;

	case BattlePhase::Movement:
		onRightClickMovementPhase(mouseDirection, mousePosition);
		break;

	case BattlePhase::Attack:
		onRightClickAttackPhase(mousePosition);
		break;
	}
}

void BattleUI::onLeftClick(sf::Vector2i mousePosition)
{
	m_leftClickHeld = true;
	m_leftClickPosition = mousePosition;
	m_shipSelector.resetShape(mousePosition);

	const Tile* tileOnMouse = m_battle.getMap().getTile(m_battle.getMap().getMouseClickCoord(mousePosition));
	if (!tileOnMouse)
	{
		m_leftClickHeld = false;
		return;
	}

	//First Click
	if (!m_tileOnLeftClick)
	{
		m_tileOnLeftClick = tileOnMouse;
		m_tileOnMouse = tileOnMouse;
	}

	BattlePhase currentBattlePhase = m_battle.getCurrentBattlePhase();

	//Clicked on new tile
	if (m_tileOnLeftClick->m_tileCoordinate != tileOnMouse->m_tileCoordinate)
	{
		if (currentBattlePhase == BattlePhase::Movement)
		{
			m_movementArea.clearTileArea();
			if (m_tileOnLeftClick->isShipOnTile())
			{
				m_battle.clearFactionShipMovementArea(m_tileOnLeftClick->m_shipOnTile);
			}
		}
		else if (currentBattlePhase == BattlePhase::Attack)
		{
			m_targetArea.clearTileArea();
		}

		m_tileOnLeftClick = tileOnMouse;
		m_tileOnMouse = tileOnMouse;
	}

	//Cancel movement for selected ships
	size_t selectedShipCount = m_shipSelector.getSelectedShips().size();
	if (selectedShipCount > 0)
	{
		for (int i = 0; i < selectedShipCount; ++i)
		{
			ShipOnTile selectedShip = m_shipSelector.removeSelectedShip();
			if (currentBattlePhase == BattlePhase::Movement)
			{
				m_battle.clearFactionShipMovementArea(selectedShip);
			}
		}
	}
}

void BattleUI::onLeftClickMovementPhase(sf::Vector2i mousePosition)
{
	if (m_shipSelector.getSelectedShips().size() == 1)
	{
		const Ship& ship = m_battle.getFactionShip(m_shipSelector.getSelectedShips()[0].m_shipOnTile);
		if (!ship.isDestinationSet())
		{
			generateMovementArea(ship);
		}
	}
	else if (m_tileOnLeftClick->isShipOnTile())
	{
		const Ship& ship = m_battle.getFactionShip(m_tileOnLeftClick->m_shipOnTile);
		if (!ship.isDestinationSet())
		{
			generateMovementArea(ship);
		}
	}
}

void BattleUI::onLeftClickAttackPhase(sf::Vector2i mousePosition)
{
	if (m_shipSelector.getSelectedShips().size() == 1)
	{
		const Ship& selectedShip = m_battle.getFactionShip(m_shipSelector.getSelectedShips()[0].m_shipOnTile);
		generateTargetArea(*m_battle.getMap().getTile(selectedShip.getCurrentPosition()));
	}
	else if (m_tileOnLeftClick->isShipOnTile() && !m_battle.getFactionShip(m_tileOnLeftClick->m_shipOnTile).isWeaponFired())
	{
		generateTargetArea(*m_tileOnLeftClick);
	}
}

void BattleUI::onRightClickAttackPhase(sf::Vector2i mousePosition)
{
	size_t selectedShipCount = m_shipSelector.getSelectedShips().size();
	if (selectedShipCount > 0)
	{
		for (int i = 0; i < selectedShipCount; ++i)
		{
			ShipOnTile selectedShip = m_shipSelector.removeSelectedShip();
			m_battle.fireFactionShipAtPosition(selectedShip, *m_tileOnRightClick, m_targetArea.m_tileArea);
		}
	}
	else
	{
		if (m_tileOnRightClick->isShipOnTile() && !m_battle.isShipBelongToFactionInPlay(m_tileOnRightClick->m_shipOnTile))
		{
			m_battle.fireFactionShipAtPosition(m_tileOnLeftClick->m_shipOnTile, *m_tileOnRightClick, m_targetArea.m_tileArea);
		}
		else if (m_tileOnLeftClick->isShipOnTile() && !m_tileOnRightClick->isShipOnTile())
		{
			m_battle.fireFactionShipAtPosition(m_tileOnLeftClick->m_shipOnTile, *m_tileOnLeftClick, m_targetArea.m_tileArea);
		}
	}

	m_targetArea.clearTileArea();
	m_tileOnLeftClick = nullptr;
	m_tileOnMouse = nullptr;
	m_tileOnRightClick = nullptr;
	m_spriteOnTileClick.deactivate();
	m_spriteOnMouse.deactivate();
}

void BattleUI::onMouseMove(sf::Vector2i mousePosition)
{
	if (m_leftClickHeld && m_battle.getCurrentBattlePhase() != BattlePhase::Deployment)
	{
		m_shipSelector.update(m_battle.getCurrentFactionShips(), mousePosition, m_battle.getMap());
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
		
		switch (m_battle.getCurrentBattlePhase())
		{
		case BattlePhase::Deployment:
		{
			onMouseMoveDeploymentPhase(mousePosition);
			break;
		}
		case BattlePhase::Movement:
		{
			onMouseMoveMovementPhase(mousePosition);
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
	else if (mousePosition.x > SCREEN_RESOLUTION.x - 100)
	{
		m_pendingCameraMovement += sf::Vector2f{ 1,0 };
	}

	if (mousePosition.y < 50)
	{
		m_pendingCameraMovement += sf::Vector2f{ 0 , -1 };
	}
	else if (mousePosition.y > SCREEN_RESOLUTION.y - 100)
	{
		m_pendingCameraMovement += sf::Vector2f{ 0, 1 };
	}
}

void BattleUI::onMouseMoveDeploymentPhase(sf::Vector2i mousePosition)
{
	//Handle whether or not to show marker that factions ship is within spawn range
	if (m_battle.getCurrentFaction().isPositionInDeploymentArea(m_tileOnMouse->m_tileCoordinate))
	{
		m_spriteOnMouse.deactivate();
	}
	else
	{
		m_spriteOnMouse.setPosition(m_tileOnMouse->m_tileCoordinate);
		m_spriteOnMouse.activate();
	}

	//Only place ship on non occupied tile
	if (!m_tileOnMouse->isShipOnTile() && !m_rightClickHeld)
	{
		m_battle.setShipDeploymentAtPosition(m_tileOnMouse->m_tileCoordinate);
	}
	else if (!m_tileOnMouse->isShipOnTile() && m_rightClickHeld)
	{
		auto mouseDirection = Math::calculateDirection(m_rightClickPosition, mousePosition);
		m_battle.setShipDeploymentAtPosition(m_tileOnRightClick->m_tileCoordinate, mouseDirection.second);
	}
}

void BattleUI::onMouseMoveMovementPhase(sf::Vector2i mousePosition)
{
	//Multiple ships selected
	if (m_shipSelector.getSelectedShips().size() > size_t(1) && !m_leftClickHeld)
	{
		m_movementArea.clearTileArea();
		while (m_movementArea.m_tileArea.size() < m_shipSelector.getSelectedShips().size())
		{
			//Fill movement area to equate to amount of ships selected
			if (m_movementArea.m_tileArea.empty())
			{
				for (int i = 0; i < m_shipSelector.getSelectedShips().size(); ++i)
				{
					const Tile* adjacentTile = m_battle.getMap().getNonCollidableAdjacentTile(
						m_movementArea.m_tileArea, m_battle.getMap().getMouseClickCoord(mousePosition));

					if (adjacentTile)
					{
						m_movementArea.m_tileArea.push_back(adjacentTile);
					}
				}
				break;
			}
		}

		assert(m_movementArea.m_tileArea.size() == m_shipSelector.getSelectedShips().size());

		//Generate ship movement paths to positions
		int shipIndex = 0;
		for (const auto& tile : m_movementArea.m_tileArea)
		{
			if (tile && !tile->isShipOnTile())
			{
				for (int i = shipIndex; i < m_shipSelector.getSelectedShips().size();)
				{
					ShipOnTile selectedShip = m_shipSelector.getSelectedShips()[i].m_shipOnTile;
					m_battle.generateFactionShipMovementArea(selectedShip, tile->m_tileCoordinate, true);
					
					++i;
					shipIndex = i;
					break;
				}
			}
		}
	}
	//Singular ship selected with ship selector
	else if (m_shipSelector.getSelectedShips().size() == 1 && !m_leftClickHeld)
	{
		ShipOnTile selectedShip = m_shipSelector.getSelectedShips()[0].m_shipOnTile;
		m_battle.generateFactionShipMovementArea(selectedShip, m_tileOnMouse->m_tileCoordinate);
	}
	//Singular ship selected with left click
	else
	{
		if (m_tileOnLeftClick && m_tileOnLeftClick->isShipOnTile() &&
			(m_tileOnMouse->m_type == eTileType::eSea || m_tileOnMouse->m_type == eTileType::eOcean))
		{
			m_battle.generateFactionShipMovementArea(m_tileOnLeftClick->m_shipOnTile, m_tileOnMouse->m_tileCoordinate);
		}
		else if (m_tileOnLeftClick && m_tileOnLeftClick->isShipOnTile())
		{
			m_battle.clearFactionShipMovementArea(m_tileOnLeftClick->m_shipOnTile);
		}
	}
}

void BattleUI::onRightClickMovementPhase(std::pair<double, eDirection> mouseDirection, sf::Vector2i mousePosition)
{	
	size_t selectedShipCount = m_shipSelector.getSelectedShips().size();
	if (selectedShipCount > 0)
	{
		for (int i = 0; i < selectedShipCount; ++i)
		{
			ShipOnTile selectedShip = m_shipSelector.removeSelectedShip();
			m_battle.moveFactionShipToPosition(selectedShip);
		}
	}
	else
	{
		if (m_tileOnLeftClick && m_tileOnLeftClick->isShipOnTile())
		{
			if (!m_tileOnRightClick->isShipOnTile() && m_movementArea.isPositionInTileArea(m_tileOnRightClick->m_tileCoordinate))
			{
				if (Math::isFacingDifferentTile(mouseDirection.first))
				{
					m_battle.moveFactionShipToPosition(m_tileOnLeftClick->m_shipOnTile, mouseDirection.second);
				}
				else
				{
					m_battle.moveFactionShipToPosition(m_tileOnLeftClick->m_shipOnTile);
				}
			}
			else
			{
				m_battle.clearFactionShipMovementArea(m_tileOnLeftClick->m_shipOnTile);
			}

			m_tileOnLeftClick = nullptr;
			m_tileOnRightClick = nullptr;
			m_movementArea.clearTileArea();
		}
	}
}

void BattleUI::onRightClick(sf::Vector2i mousePosition)
{
	m_leftClickHeld = false;
	m_rightClickHeld = true;
	m_rightClickPosition = mousePosition;

	const Tile* tileOnMouse = m_battle.getMap().getTile(m_battle.getMap().getMouseClickCoord(mousePosition));
	if (tileOnMouse)
	{
		m_tileOnRightClick = tileOnMouse;
	}
	else
	{
		m_rightClickHeld = false;
	}
}

void BattleUI::onRightClickDeploymentPhase(eDirection startingDirection)
{
	if ((m_tileOnRightClick->m_type == eTileType::eSea || m_tileOnRightClick->m_type == eTileType::eOcean))
	{
		//If tile isn't already occupied by a ship
		if (!m_tileOnRightClick->isShipOnTile())
		{
			m_spriteOnTileClick.setPosition(m_tileOnRightClick->m_tileCoordinate);
			m_battle.deployFactionShipAtPosition(m_tileOnRightClick->m_tileCoordinate, startingDirection);
		}
	}
}

void BattleUI::onCancelMovementPhase(sf::Vector2i mousePosition)
{
	//Cancel selected ships within box
	if (!m_shipSelector.getSelectedShips().empty())
	{
		for (auto& selectedShip : m_shipSelector.getSelectedShips())
		{
			if (selectedShip.m_shipOnTile.isValid())
			{
				m_battle.clearFactionShipMovementArea(selectedShip.m_shipOnTile);
			}
		}
	}
	//Cancel individually selected Ship
	else if (m_tileOnLeftClick && m_tileOnLeftClick->isShipOnTile())
	{
		m_battle.clearFactionShipMovementArea(m_tileOnLeftClick->m_shipOnTile); 
		m_spriteOnTileClick.deactivate();
	}

	m_tileOnLeftClick = nullptr;
	m_tileOnRightClick = nullptr;
	m_movementArea.clearTileArea();
}

void BattleUI::onCancelAttackPhase(sf::Vector2i mousePosition)
{
	m_tileOnLeftClick = nullptr;
	m_spriteOnTileClick.deactivate();
	m_targetArea.clearTileArea();
}

void BattleUI::onMouseMoveAttackPhase(sf::Vector2i mousePosition)
{
	//Multiple ships selected
	if (m_shipSelector.getSelectedShips().size() > 1)
	{
		m_spriteOnMouse.setPosition(m_tileOnMouse->m_tileCoordinate);
		m_spriteOnMouse.activate();
	}
	//Singular ship selected
	else
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
}

void BattleUI::onNewBattlePhase(GameEvent gameEvent)
{
	m_tileOnRightClick = nullptr;
	m_tileOnLeftClick = nullptr;
	m_tileOnMouse = nullptr;
	m_leftClickHeld = false;
	m_rightClickHeld = false;
	m_spriteOnTileClick.deactivate();
	m_targetArea.clearTileArea();
	m_movementArea.clearTileArea();
	m_shipSelector.reset();
}