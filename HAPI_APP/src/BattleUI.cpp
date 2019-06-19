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
	m_tileOnPreviousClick(nullptr),
	m_tileOnClick(nullptr),
	m_tileOnMouse(nullptr),
	m_tileOnClickSprite(),
	m_spriteOnMouse(Textures::getInstance().m_cross, false),
	m_shipMovementArea(Textures::getInstance().m_selectedHex, MAX_MOVE_AREA, m_battle.getMap()),
	m_shipTargetArea(Textures::getInstance().m_mouseCrossHair, MAX_TARGET_AREA, m_battle.getMap()),
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
	m_tileOnClickSprite.render(window, m_battle.getMap());
	m_shipMovementArea.render(window, m_battle.getMap());
	m_battle.renderFactionShipsMovementGraphs(window);

	for (auto& i : m_shipTargetArea.m_tileAreaGraph)
	{
		i.render(window, m_battle.getMap());
	}

	if (m_tileOnClick && (m_tileOnClick->m_type == eTileType::eSea || m_tileOnClick->m_type == eTileType::eOcean))
	{
		m_tileOnClickSprite.render(window, m_battle.getMap());
	}

	m_spriteOnMouse.render(window, m_battle.getMap());
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
	case sf::Event::MouseButtonPressed :
		if (currentEvent.mouseButton.button == sf::Mouse::Left)
		{
			onLeftClick(mousePosition);
		}
		else if (currentEvent.mouseButton.button == sf::Mouse::Right && currentPlayerType == ePlayerType::eHuman)
		{
			onRightClick(mousePosition);
		}
		break;

	case sf::Event::MouseMoved :
		onMouseMovement(mousePosition);
		break;

	case sf::Event::KeyPressed :
		if (currentPlayerType == ePlayerType::eHuman)
		{
			GameEventMessenger::getInstance().broadcast(GameEvent(), eGameEvent::eEndBattlePhaseEarly);
		}
		break;

	case sf::Event::MouseButtonReleased :
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
	m_shipTargetArea.clearTileArea();

	const Ship& ship = m_battle.getFactionShip(source.m_shipOnTile);
	if (ship.getShipType() == eShipType::eFrigate)
	{
		m_battle.getMap().getTileCone(m_shipTargetArea.m_tileArea, source.m_tileCoordinate,
			ship.getRange(),
			ship.getCurrentDirection(), true);

	}
	else if (ship.getShipType() == eShipType::eSniper)
	{
		m_battle.getMap().getTileLine(m_shipTargetArea.m_tileArea, source.m_tileCoordinate,
			ship.getRange(),
			ship.getCurrentDirection(), true);

	}
	else if (ship.getShipType() == eShipType::eTurtle)
	{
		// make so where ever the place presses get radius called talk adrais about size of that
		m_battle.getMap().getTileRadius(m_shipTargetArea.m_tileArea, source.m_tileCoordinate,
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

		m_battle.getMap().getTileLine(m_shipTargetArea.m_tileArea, source.m_tileCoordinate,
			ship.getRange(), directionOfFire, true);	
	}

	m_shipTargetArea.activateGraph();
}

void BattleUI::generateMovementArea(const Ship & ship)
{
	m_shipMovementArea.clearTileArea();

	posi startPos = { ship.getCurrentPosition(), ship.getCurrentDirection() };
	BFS::findArea(m_shipMovementArea.m_tileArea, m_battle.getMap(), startPos, ship.getMovementPoints());

	m_shipMovementArea.activateGraph();
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
	if (m_battle.getCurrentBattlePhase() == BattlePhase::Deployment 
		|| m_battle.getCurrentBattlePhase() == BattlePhase::Movement)
	{
		m_leftClickHeld = true;
	}
	
	m_leftClickPosition = mousePosition;

	const Tile* tileOnMouse = m_battle.getMap().getTile(m_battle.getMap().getMouseClickCoord(mousePosition));
	if (!tileOnMouse)
	{
		return;
	}

	//First Click
	if (!m_tileOnClick)
	{
		m_tileOnClick = tileOnMouse;
		m_tileOnMouse = tileOnMouse;
	}

	//Clicked on new tile
	if (m_tileOnClick->m_tileCoordinate != tileOnMouse->m_tileCoordinate)
	{
		m_tileOnPreviousClick = m_tileOnClick;
		m_tileOnClick = tileOnMouse;
		m_tileOnMouse = tileOnMouse;
	}

	//Clicked on tile containing non usable ship
	if (m_tileOnClick->isShipOnTile())
	{
		auto& ship = m_battle.getFactionShip(m_tileOnClick->m_shipOnTile);
		if (ship.isDead())
		{
			m_tileOnPreviousClick = nullptr;
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
	if (!m_tileOnPreviousClick && m_tileOnClick->isShipOnTile() && !m_battle.getFactionShip(m_tileOnClick->m_shipOnTile).isWeaponFired()
		&& m_battle.isShipBelongToCurrentFactionInPlay(m_tileOnClick->m_shipOnTile))
	{
		generateTargetArea(*m_tileOnClick);
		return;
	}

	if (m_tileOnClick->isShipOnTile() && m_tileOnPreviousClick && 
		!m_battle.isShipBelongToCurrentFactionInPlay(m_tileOnClick->m_shipOnTile))
	{
		m_battle.fireFactionShipAtPosition(m_tileOnPreviousClick->m_shipOnTile, *m_tileOnClick, m_shipTargetArea.m_tileArea);
	}
	else if (!m_tileOnClick->isShipOnTile() && m_tileOnPreviousClick)
	{
		m_battle.fireFactionShipAtPosition(m_tileOnPreviousClick->m_shipOnTile, *m_tileOnClick, m_shipTargetArea.m_tileArea);
	}

	m_shipTargetArea.clearTileArea();
	m_tileOnClick = nullptr;
	m_tileOnPreviousClick = nullptr;
	m_tileOnMouse = nullptr;
	m_tileOnClickSprite.deactivate();
	m_spriteOnMouse.deactivate();
}

void BattleUI::onMouseMovement(sf::Vector2i mousePosition)
{
	moveCamera(mousePosition);

	//Ship selected doesn't match faction currently in play
	if ((m_tileOnClick && m_tileOnClick->isShipOnTile()
		&& !m_battle.isShipBelongToCurrentFactionInPlay(m_tileOnClick->m_shipOnTile))
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
	if ((m_tileOnClick->m_type == eTileType::eSea || m_tileOnClick->m_type == eTileType::eOcean))
	{
		//If tile isn't already occupied by a ship
		if (!m_tileOnClick->isShipOnTile())
		{
			m_tileOnClickSprite.setPosition(m_tileOnClick->m_tileCoordinate);
			m_battle.deployFactionShipAtPosition(m_tileOnClick->m_tileCoordinate, startingDirection);
		}
	}
}

void BattleUI::onMouseMoveMovementPhase(sf::Vector2i mousePosition)
{
	if (m_tileOnClick && m_tileOnClick->isShipOnTile() &&
		(m_tileOnMouse->m_type == eTileType::eSea || m_tileOnMouse->m_type == eTileType::eOcean))
	{
		m_battle.generateFactionShipMovementGraph(m_tileOnClick->m_shipOnTile, m_tileOnMouse->m_tileCoordinate);
	}
	else if(m_tileOnClick && m_tileOnClick->isShipOnTile())
	{
		m_battle.disableFactionShipMovementGraph(m_tileOnClick->m_shipOnTile);
	}
}

void BattleUI::onLeftClickMovementPhase(std::pair<double, eDirection> mouseDirection, sf::Vector2i mousePosition)
{	
	//On first Ship Selection
	if (!m_tileOnPreviousClick && m_tileOnClick->isShipOnTile() && 
		m_battle.isShipBelongToCurrentFactionInPlay(m_tileOnClick->m_shipOnTile))
	{
		const Ship& ship = m_battle.getFactionShip(m_tileOnClick->m_shipOnTile);
		if (!ship.isDestinationSet())
		{
			generateMovementArea(ship);
		}
	}
	//Ship already selected
	else if(m_tileOnPreviousClick && m_tileOnPreviousClick->isShipOnTile())
	{	
		if (!m_tileOnClick->isShipOnTile() && m_shipMovementArea.isPositionInTileArea(m_tileOnClick->m_tileCoordinate))
		{
			if (Math::isFacingDifferentTile(mouseDirection.first))
			{
				m_battle.moveFactionShipToPosition(m_tileOnPreviousClick->m_shipOnTile, m_tileOnMouse->m_tileCoordinate, mouseDirection.second);
			}
			else
			{
				m_battle.moveFactionShipToPosition(m_tileOnPreviousClick->m_shipOnTile, m_tileOnMouse->m_tileCoordinate);
			}
		}
		else if (!m_tileOnClick->isShipOnTile() && !m_shipMovementArea.isPositionInTileArea(m_tileOnClick->m_tileCoordinate))
		{
			m_battle.disableFactionShipMovementGraph(m_tileOnPreviousClick->m_shipOnTile);
		}
		else if (m_tileOnClick->isShipOnTile())
		{
			m_battle.disableFactionShipMovementGraph(m_tileOnPreviousClick->m_shipOnTile);
		}

		m_tileOnClick = nullptr;
		m_tileOnPreviousClick = nullptr;
		m_shipMovementArea.clearTileArea();
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
}

void BattleUI::onRightClickMovementPhase(sf::Vector2i mousePosition)
{
	//Cancel selected Entity
	if (m_tileOnClick && m_tileOnClick->isShipOnTile())
	{
		m_battle.disableFactionShipMovementGraph(m_tileOnClick->m_shipOnTile); 
		m_tileOnClickSprite.deactivate();
	}
	//TODO: Drop info box
	m_tileOnClick = nullptr;
	m_tileOnPreviousClick = nullptr;
	m_shipMovementArea.clearTileArea();
}

void BattleUI::onRightClickAttackPhase(sf::Vector2i mousePosition)
{
	m_tileOnClick = nullptr;
	m_tileOnClickSprite.deactivate();
	m_shipTargetArea.clearTileArea();
}

void BattleUI::onMouseMoveAttackPhase(sf::Vector2i mousePosition)
{
	auto tileCoordinate = m_tileOnMouse->m_tileCoordinate;
	auto cIter = std::find_if(m_shipTargetArea.m_tileArea.cbegin(), m_shipTargetArea.m_tileArea.cend(),
		[tileCoordinate](const auto& tile) { return tileCoordinate == tile->m_tileCoordinate; });
	
	if (cIter != m_shipTargetArea.m_tileArea.cend())
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
	m_tileOnClick = nullptr;
	m_tileOnPreviousClick = nullptr;
	m_tileOnMouse = nullptr;
	m_tileOnClickSprite.deactivate();
	m_shipTargetArea.clearTileArea();
	m_shipMovementArea.clearTileArea();
}