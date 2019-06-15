#include "BattleUI.h"
#include "Battle.h"
#include "BFS.h"
#include "Textures.h"
#include "MouseSelection.h"
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
	m_gui(),
	m_shipMovementArea(Textures::getInstance().m_selectedHex, MAX_MOVE_AREA, m_battle.getMap()),
	m_shipTargetArea(Textures::getInstance().m_mouseCrossHair, MAX_TARGET_AREA, m_battle.getMap())
{
	GameEventMessenger::getInstance().subscribe(std::bind(&BattleUI::onNewBattlePhase, this), GameEvent::eEnteredNewBattlePhase);
}

BattleUI::~BattleUI()
{
	GameEventMessenger::getInstance().unsubscribe(GameEvent::eEnteredNewBattlePhase);
}

sf::Vector2i BattleUI::getCameraPositionOffset() const
{
	return m_gui.getCameraPositionOffset();
}

void BattleUI::renderUI(sf::RenderWindow& window)
{
	renderTileHighlight(window);
	m_shipMovementArea.render(window, m_battle.getMap());
}

void BattleUI::loadGUI(sf::Vector2i mapDimensions)
{
	m_gui.setMaxCameraOffset(mapDimensions);
}

void BattleUI::renderTargetArea(sf::RenderWindow& window)
{
	for (auto& i : m_shipTargetArea.m_tileAreaGraph)
	{
		i.render(window, m_battle.getMap());	
	}
}

void BattleUI::handleInput(const sf::RenderWindow& window, const sf::Event & currentEvent)
{
	sf::Vector2i mousePosition = sf::Mouse::getPosition(window);

	if (currentEvent.type == sf::Event::MouseButtonPressed)
	{
		if (currentEvent.mouseButton.button == sf::Mouse::Left)
		{
			onLeftClick(mousePosition);
		}
		else if (currentEvent.mouseButton.button == sf::Mouse::Right)
		{
			onRightClick(mousePosition);
		}
	}
	else if (currentEvent.type == sf::Event::MouseMoved)
	{
		m_gui.onMouseMove(mousePosition);
		onMouseMovement(mousePosition);
	}
	else if (currentEvent.type == sf::Event::KeyPressed)
	{
		GameEventMessenger::getInstance().broadcast(GameEvent::eEndBattlePhaseEarly);
	}
}

void BattleUI::update(float deltaTime)
{
	m_gui.update();
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

void BattleUI::onLeftClick(sf::Vector2i mousePosition)
{
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
		if (ship.isDead() || ship.getFactionName() != m_battle.getCurrentFaction())
		{
			m_tileOnPreviousClick = nullptr;
			return;
		}
	}

	switch (m_battle.getCurrentPhase())
	{
	case BattlePhase::Deployment:
	{
		onLeftClickDeploymentPhase(); 
		break;
	}
	case BattlePhase::Movement:
	{
		onLeftClickMovementPhase(mousePosition);
		break;
	}
	case BattlePhase::Attack:
	{
		onLeftClickAttackPhase(mousePosition);
		break;
	}
	}
}

void BattleUI::onMouseMovement(sf::Vector2i mousePosition)
{
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

		switch (m_battle.getCurrentPhase())
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

void BattleUI::onMouseMoveDeploymentPhase(sf::Vector2i mousePosition)
{
	//Only place ship on non occupied tile
	if (!m_tileOnMouse->m_shipOnTile.isValid())
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
			m_tileHighlight.setPosition(m_tileOnClick->m_tileCoordinate);
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
	else if(m_tileOnClick)
	{
		m_battle.disableFactionShipMovementGraph(m_tileOnClick->m_shipOnTile);
	}
}

void BattleUI::onLeftClickMovementPhase(sf::Vector2i mousePosition)
{	
	//On first Ship Selection
	if (!m_tileOnPreviousClick && m_tileOnClick->isShipOnTile())
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
		//Ship not present on tile selected
		if (!m_tileOnClick->isShipOnTile())
		{
			m_battle.moveFactionShipToPosition(m_tileOnPreviousClick->m_shipOnTile, m_tileOnMouse->m_tileCoordinate);
			m_tileOnClick = nullptr;
			m_tileOnPreviousClick = nullptr;
			m_shipMovementArea.clearTileArea();
		}
	}
}

void BattleUI::onRightClick(sf::Vector2i mousePosition)
{
	switch (m_battle.getCurrentPhase())
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
		m_tileHighlight.deactivate();
	}
	//TODO: Drop info box
	m_tileOnClick = nullptr;
	m_tileOnPreviousClick = nullptr;
	m_shipMovementArea.clearTileArea();
}

void BattleUI::onLeftClickAttackPhase(sf::Vector2i mousePosition)
{
	if (m_tileOnClick->isShipOnTile() && !m_battle.getFactionShip(m_tileOnMouse->m_shipOnTile).isWeaponFired())
	{
		generateTargetArea(*m_tileOnMouse);	
	}

	//if (!m_tileOnClick && tileOnMouse->m_shipOnTile.isValid() && m_battle.getFactionShip(tileOnMouse->m_shipOnTile).getFactionName() != m_battle.getCurrentFaction())
	//{
	//	m_tileOnClick = tileOnMouse;
	//	return;
	//}

	////Do not fire on destroyed ship
	//if (tileOnMouse->m_shipOnTile.isValid() && m_battle.getFactionShip(tileOnMouse->m_shipOnTile).isDead())
	//{
	//	m_targetArea.clearTileArea();
	//	m_tileHighlight.deactivate();
	//	//TODO: Drop info box
	//	m_tileOnClick = nullptr;
	//	return;
	//}

	////Clicking on the same entity that has been previously selected
	//if (m_tileOnClick && m_tileOnClick->m_tileCoordinate == tileOnMouse->m_tileCoordinate)
	//{
	//	m_targetArea.clearTileArea();
	//	m_tileHighlight.deactivate();
	//	//TODO: Drop info box
	//	m_tileOnClick = nullptr;
	//	return;
	//}

	////Entity already selected Fire weapon at position
	////If the selectedTile has an entity on it and that entity hasn't fired
	//if (m_tileOnClick && m_tileOnClick->isShipOnTile() && 
	//	!m_battle.getFactionShip(m_tileOnClick->m_shipOnTile).isWeaponFired())
	//{
	//	//If there is an entity on the tile you're clicking on and that entity's faction name differs from the one of the ship that's firing
	//	if ((tileOnMouse->m_shipOnTile.isValid()) && m_battle.getFactionShip(tileOnMouse->m_shipOnTile).getFactionName() != 
	//		m_battle.getFactionShip(m_tileOnClick->m_shipOnTile).getFactionName())
	//	{
	//		m_battle.fireFactionShipAtPosition(m_tileOnClick->m_shipOnTile, tileOnMouse->m_shipOnTile, m_targetArea.m_tileArea);
	//	}
	//	m_targetArea.clearTileArea();
	//	m_tileOnClick = nullptr;
	//	m_tileHighlight.deactivate();
	//	return;
	//}

	////Entity Already Selected whilst showing where to fire
	////Change to different Entity before firing
	//if (tileOnMouse->m_shipOnTile.isValid() && m_battle.getFactionShip(tileOnMouse->m_shipOnTile).getFactionName() == m_battle.getCurrentFaction()
	//	 && m_tileOnClick && m_targetArea.m_tileArea.size() > 0)
	//{
	//	m_targetArea.clearTileArea();
	//	generateTargetArea(*tileOnMouse);
	//	//TODO: Raise info box
	//	m_tileOnClick = tileOnMouse;
	//	return;
	//}

	////Click on same
	////Select new Entity to fire at something
	//if (tileOnMouse->m_shipOnTile.isValid() && m_battle.getFactionShip(tileOnMouse->m_shipOnTile).getFactionName() != m_battle.getCurrentFaction())
	//{
	//	m_tileOnClick = tileOnMouse;
	//	return;
	//}

	//if (!m_tileOnClick && tileOnMouse->m_shipOnTile.isValid() && m_battle.getFactionShip(tileOnMouse->m_shipOnTile).getFactionName() == 
	//	m_battle.getCurrentFaction())
	//{
	//	m_tileOnClick = tileOnMouse;
	//}

	//if (m_tileOnClick && m_tileOnClick->isShipOnTile() && 
	//	!m_battle.getFactionShip(m_tileOnClick->m_shipOnTile).isWeaponFired() &&
	//	m_battle.getFactionShip(m_tileOnClick->m_shipOnTile).getFactionName() == m_battle.getCurrentFaction())
	//{
	//	m_tileOnClick = tileOnMouse;
	//	generateTargetArea(*tileOnMouse);
	//}
}

void BattleUI::onRightClickAttackPhase(sf::Vector2i mousePosition)
{
	//TODO: Drop info box
	m_tileOnClick = nullptr;
	m_tileHighlight.deactivate();
	m_shipTargetArea.clearTileArea();
}

void BattleUI::onMouseMoveAttackPhase(sf::Vector2i mousePosition)
{
	auto tileCoordinate = m_tileOnMouse->m_tileCoordinate;
	//tileOnMouse in new position
	if (m_tileOnClick && m_tileOnClick->m_tileCoordinate != m_tileOnMouse->m_tileCoordinate)
	{
		//TODO: this does not work if some of the tiles are nullptr!
		auto cIter = std::find_if(m_shipTargetArea.m_tileArea.cbegin(), m_shipTargetArea.m_tileArea.cend(),
			[tileCoordinate](const auto& tile) { 
				if (tile != nullptr)
				{
					return tileCoordinate == tile->m_tileCoordinate;
				}
			});
		//tileOnMouse within weapon range
		if (cIter != m_shipTargetArea.m_tileArea.cend())
		{
			m_tileHighlight.setPosition(m_tileOnMouse->m_tileCoordinate, m_battle.getMap());
			m_tileHighlight.activate();
		}
		//outside weapon range
		else
		{
			m_tileHighlight.deactivate();
		}
	}
}

void BattleUI::onNewBattlePhase()
{
	m_tileOnClick = nullptr;
	m_tileOnPreviousClick = nullptr;
	m_tileOnMouse = nullptr;
	m_tileHighlight.deactivate();
	m_shipTargetArea.clearTileArea();
	m_shipMovementArea.clearTileArea();
}