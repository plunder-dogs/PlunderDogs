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

constexpr size_t MAX_MOVE_AREA_GRAPH{ 700 };
constexpr size_t MAX_TARGET_AREA_GRAPH = 50;

BattleUI::BattleUI(Battle & battle)
	: m_battle(battle),
	m_tileOnPreviousClick(nullptr),
	m_tileOnClick(nullptr),
	m_tileOnMouse(nullptr),
	m_gui(),
	m_movementArea(Textures::getInstance().m_selectedHex, MAX_MOVE_AREA_GRAPH, m_battle.getMap()),
	m_targetArea(Textures::getInstance().m_mouseCrossHair, MAX_TARGET_AREA_GRAPH, m_battle.getMap())
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
	m_tileHighlight.render(window, m_battle.getMap());
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

void BattleUI::drawTargetArea(sf::RenderWindow& window)
{
	for (auto& i : m_targetArea.m_tileAreaGraph)
	{
		if (i.isActive())
		{
			i.render(window, m_battle.getMap());
		}
	}
}

void BattleUI::handleInput(sf::RenderWindow& window, const sf::Event & currentEvent)
{
	sf::Vector2i mousePosition = sf::Mouse::getPosition(window);

	if (currentEvent.type == sf::Event::MouseButtonPressed)
	{
		if (currentEvent.mouseButton.button == sf::Mouse::Left)
		{
			onLeftClick(window);
		}
		else if (currentEvent.mouseButton.button == sf::Mouse::Right)
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
	}
	else if (currentEvent.type == sf::Event::MouseMoved)
	{
		if (m_battle.getCurrentPlayerType() == ePlayerType::eAI)
		{
			return;
		}

		m_gui.onMouseMove(sf::Mouse::getPosition(window));
		handleMouseMovement(sf::Mouse::getPosition(window));
	}
}

void BattleUI::update(float deltaTime)
{
	m_gui.update();
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

	if (m_targetArea.m_tileArea.empty())
	{
		return;
	}

	for (int i = 0; i < m_targetArea.m_tileArea.size(); i++)
	{
		sf::Vector2i tilePos = m_battle.getMap().getTileScreenPos(m_targetArea.m_tileArea[i]->m_tileCoordinate);
		m_targetArea.m_tileAreaGraph[i].setPosition(sf::Vector2i(
			tilePos.x + 12 * m_battle.getMap().getDrawScale(),
			tilePos.y + 28 * m_battle.getMap().getDrawScale()
		));

		m_targetArea.m_tileAreaGraph[i].activate();
		//TODO: Not sure if need this line
		//m_targetArea.m_tileAreaGraph[i].setPosition(m_targetArea[i]->m_tileCoordinate);
	}
}

void BattleUI::generateMovementArea(const Ship & ship)
{
	m_movementArea.clearTileArea();
	m_movementArea.m_display = true;

	posi startPos = { ship.getCurrentPosition(), ship.getCurrentDirection() };
	BFS::findArea(m_movementArea.m_tileArea, m_battle.getMap(), startPos, ship.getMovementPoints());
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
	sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
	const Tile* tileOnMouse = m_battle.getMap().getTile(m_battle.getMap().getMouseClickCoord(mousePosition));
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
		m_tileOnMouse = tileOnMouse;
	}

	//if(m_tileOnClick->m_shipOnTile.isValid() && )
	//Back out when selecting a destroyed ship?
	//Back out when selecting an enemy ship?

	switch (m_battle.getCurrentPhase())
	{
	case BattlePhase::Deployment:
	{
		onLeftClickDeploymentPhase();
		m_tileHighlight.deactivate();
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

void BattleUI::handleMouseMovement(sf::Vector2i mousePosition)
{
	//Ship selected doesn't match faction currently in play
	if (m_tileOnClick && m_tileOnClick->m_shipOnTile.isValid()
		&& m_battle.getFactionShip(m_tileOnClick->m_shipOnTile).getFactionName() != m_battle.getCurrentFaction())
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

	m_tileHighlight.setPosition(m_tileOnMouse->m_tileCoordinate);
	m_tileHighlight.activate();
}

void BattleUI::onLeftClickDeploymentPhase(eDirection startingDirection)
{
	if ((m_tileOnClick->m_type == eTileType::eSea || m_tileOnClick->m_type == eTileType::eOcean))
	{
		//If tile isn't already occupied by a ship
		if (!m_tileOnClick->m_shipOnTile.isValid())
		{
			m_tileHighlight.setPosition(m_tileOnClick->m_tileCoordinate);
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
			m_tileHighlight.deactivate();
		}
		else
		{ 
			m_tileHighlight.setPosition(m_tileOnMouse->m_tileCoordinate, m_battle.getMap());
			m_tileHighlight.activate();
			m_battle.disableFactionShipMovementGraph(m_tileOnClick->m_shipOnTile);
		}	
	}
}

void BattleUI::onLeftClickMovementPhase(sf::Vector2i mousePosition)
{	
	//On first Ship Selection
	if (!m_tileOnPreviousClick)
	{
		//m_battle.disableFactionShipMovementGraph(m_tileOnClick->m_shipOnTile); //TODO: Not sure if need
		const Ship& ship = m_battle.getFactionShip(m_tileOnClick->m_shipOnTile);
		if (!ship.isDestinationSet())
		{
			generateMovementArea(ship);
		}
	}
	//Ship already selected
	else if (m_tileOnPreviousClick && m_tileOnPreviousClick->m_shipOnTile.isValid())
	{	
		//Ship not present on tile selected
		if (!m_tileOnClick->m_shipOnTile.isValid())
		{
			m_battle.moveFactionShipToPosition(m_tileOnPreviousClick->m_shipOnTile, m_tileOnMouse->m_tileCoordinate);
			m_tileOnClick = nullptr;
			m_tileOnPreviousClick = nullptr;
		}
	}
}

void BattleUI::onRightClickMovementPhase(sf::Vector2i mousePosition)
{
	//Cancel selected Entity
	if (m_tileOnClick && m_tileOnClick->m_shipOnTile.isValid())
	{
		m_battle.disableFactionShipMovementGraph(m_tileOnClick->m_shipOnTile); 
		m_tileHighlight.deactivate();
	}
	//TODO: Drop info box
	m_tileOnClick = nullptr;
	m_tileOnPreviousClick = nullptr;
	m_movementArea.clearTileArea();
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
			m_targetArea.clearTileArea();
			generateTargetArea(*tileOnMouse);
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
		m_targetArea.clearTileArea();
		m_tileHighlight.deactivate();
		//TODO: Drop info box
		m_tileOnClick = nullptr;
		return;
	}

	//Clicking on the same entity that has been previously selected
	if (m_tileOnClick && m_tileOnClick->m_tileCoordinate == tileOnMouse->m_tileCoordinate)
	{
		m_targetArea.clearTileArea();
		m_tileHighlight.deactivate();
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
			m_battle.fireFactionShipAtPosition(m_tileOnClick->m_shipOnTile, tileOnMouse->m_shipOnTile, m_targetArea.m_tileArea);
		}
		m_targetArea.clearTileArea();
		m_tileOnClick = nullptr;
		m_tileHighlight.deactivate();
		return;
	}

	//Entity Already Selected whilst showing where to fire
	//Change to different Entity before firing
	if (tileOnMouse->m_shipOnTile.isValid() && m_battle.getFactionShip(tileOnMouse->m_shipOnTile).getFactionName() == m_battle.getCurrentFaction()
		 && m_tileOnClick && m_targetArea.m_tileArea.size() > 0)
	{
		m_targetArea.clearTileArea();
		generateTargetArea(*tileOnMouse);
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
		generateTargetArea(*tileOnMouse);
	}
}

void BattleUI::onRightClickAttackPhase(sf::Vector2i mousePosition)
{
	//TODO: Drop info box
	m_tileOnClick = nullptr;
	m_tileHighlight.deactivate();
	m_targetArea.clearTileArea();
}

void BattleUI::onMouseMoveAttackPhase(sf::Vector2i mousePosition)
{
	auto tileCoordinate = m_tileOnMouse->m_tileCoordinate;
	//tileOnMouse in new position
	if (m_tileOnClick && m_tileOnClick->m_tileCoordinate != m_tileOnMouse->m_tileCoordinate)
	{
		//TODO: this does not work if some of the tiles are nullptr!
		auto cIter = std::find_if(m_targetArea.m_tileArea.cbegin(), m_targetArea.m_tileArea.cend(),
			[tileCoordinate](const auto& tile) { 
				if (tile != nullptr)
				{
					return tileCoordinate == tile->m_tileCoordinate;
				}
			});
		//tileOnMouse within weapon range
		if (cIter != m_targetArea.m_tileArea.cend())
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
	m_targetArea.clearTileArea();
	m_movementArea.clearTileArea();
}