#include "BattleUI.h"
#include "Battle.h"
#include "PathFinding.h"
#include "Resources.h"
#include "Utilities/Utilities.h"
#include "GameEventMessenger.h"
#include "NetworkHandler.h"
#include <assert.h>
#include "AIHandler.h"

//Debug
#include <iostream>

constexpr size_t MAX_MOVE_AREA{ 850 };
constexpr size_t MAX_TARGET_AREA = 50;
const sf::Vector2f CAMERA_MOVE_SPEED{ 2.0f, 2.0f };
constexpr int MINIMUM_MULTIPLE_SHIP_SEARCH_AREA = 1;
constexpr int CAMERA_MOVEMENT_BOUNDARY = 10;

BattleUI::BattleUI(Battle & battle)
	: m_battle(battle),
	m_tileOnLeftClick(nullptr),
	m_tileOnMouse(nullptr),
	m_spriteOnTileClick(),
	m_spriteOnMouse(Textures::getInstance().getTexture(CROSS), false),
	m_movementArea(Textures::getInstance().getTexture(SELECTED_HEX), MAX_MOVE_AREA),
	m_targetArea(Textures::getInstance().getTexture(CROSSHAIR), MAX_TARGET_AREA),
	m_leftClickHeld(false),
	m_leftClickPosition(),
	m_cameraBounds(),
	m_cameraVelocity(),
	m_cameraPosition(),
	m_battleUILayer(eGameState::eBattle)
{
	//Battle - UI
	std::vector<UIComponentButton> battleButtons;
	battleButtons.emplace_back(Textures::getInstance().getTexture("EndPhaseButtons.xml"), sf::Vector2i(200, 800), eUIComponentName::eEndPhase, 
		eIsComponentChangeOnIntersect::eTrue, eIsComponentVisible::eFalse);
	battleButtons.emplace_back(Textures::getInstance().getTexture("pauseButton.xml"), sf::Vector2i(1400, 200), eUIComponentName::ePause, eIsComponentChangeOnIntersect::eTrue);
	m_battleUILayer.setButtons(std::move(battleButtons));
	std::vector<UIComponentImage> battleImages;
	battleImages.emplace_back(Textures::getInstance().getTexture("playerFlags.xml"), sf::Vector2i(500, 100), eUIComponentName::eFactionFlags);
	battleImages.emplace_back(Textures::getInstance().getTexture("shipStats.xml"), sf::Vector2i(400, 850), eUIComponentName::eShipStats, eIsComponentVisible::eFalse);
	m_battleUILayer.setImages(std::move(battleImages));
	
	const auto& font = Fonts::getInstance().getFont("arial");
	std::vector<UIComponentTextBox> textBoxes;
	textBoxes.reserve(size_t(4));
	textBoxes.emplace_back("", font, sf::Vector2i(550, 875), eUIComponentName::eShipStatHealth, eIsComponentVisible::eFalse, sf::Color::Black);
	textBoxes.emplace_back("", font, sf::Vector2i(750, 875), eUIComponentName::eShipStatAttackRange, eIsComponentVisible::eFalse, sf::Color::Black);
	textBoxes.emplace_back("", font, sf::Vector2i(950, 875), eUIComponentName::eShipStatMovementRange, eIsComponentVisible::eFalse, sf::Color::Black);
	textBoxes.emplace_back("", font, sf::Vector2i(1150, 875), eUIComponentName::eShipStatDamage, eIsComponentVisible::eFalse, sf::Color::Black);
	m_battleUILayer.setTextBoxes(std::move(textBoxes));

	GameEventMessenger::getInstance().subscribe(std::bind(&BattleUI::onNewBattlePhase, this, std::placeholders::_1), eGameEvent::eEnteredNewBattlePhase);
}

BattleUI::~BattleUI()
{
	GameEventMessenger::getInstance().unsubscribe(eGameEvent::eEnteredNewBattlePhase);
}

TileArea & BattleUI::getTargetArea()
{
	return m_targetArea;
}

sf::Vector2i BattleUI::getCameraPosition() const
{
	return m_cameraPosition;
}

void BattleUI::onNewFactionTurn()
{
	switch (m_battle.getCurrentFaction().m_factionName)
	{
	case eFactionName::eYellow:
		m_battleUILayer.setComponentFrameID(eUIComponentName::eFactionFlags, eUIComponentType::eImage, 0);
		break;
	case eFactionName::eBlue:
		m_battleUILayer.setComponentFrameID(eUIComponentName::eFactionFlags, eUIComponentType::eImage, 1);
		break;
	case eFactionName::eGreen:
		m_battleUILayer.setComponentFrameID(eUIComponentName::eFactionFlags, eUIComponentType::eImage, 2);
		break;
	case eFactionName::eRed:
		m_battleUILayer.setComponentFrameID(eUIComponentName::eFactionFlags, eUIComponentType::eImage, 3);
		break;
	}
}

void BattleUI::hideEndPhaseButton()
{
	m_battleUILayer.setComponentVisibility(eUIComponentName::eEndPhase, eUIComponentType::eButton, false);
}

void BattleUI::showEndPhaseButton()
{
	m_battleUILayer.setComponentVisibility(eUIComponentName::eEndPhase, eUIComponentType::eButton, true);
}

void BattleUI::render(sf::RenderWindow& window)
{
	m_spriteOnTileClick.render(window, m_battle.getMap());
	m_movementArea.render(window, m_battle.getMap());
	m_battle.renderFactionShipsMovementGraph(window);
	m_targetArea.render(window, m_battle.getMap());

	if (m_tileOnLeftClick && (m_tileOnLeftClick->m_type == eTileType::eSea || m_tileOnLeftClick->m_type == eTileType::eOcean))
	{
		m_spriteOnTileClick.render(window, m_battle.getMap());
	}

	m_spriteOnMouse.render(window, m_battle.getMap());
	m_shipSelector.render(window, m_battle.getMap(), m_leftClickHeld);
	m_battleUILayer.render(window);
}

void BattleUI::setCameraBounds(sf::Vector2i mapDimensions)
{
	m_cameraBounds.left = -120;
	m_cameraBounds.top = -100;
	m_cameraBounds.width = mapDimensions.x * 24 - 820;
	m_cameraBounds.height = mapDimensions.y * 28 - 400;
}

void BattleUI::handleInput(const sf::Event & currentEvent, sf::Vector2i mousePosition)
{
	switch (currentEvent.type)
	{
	case sf::Event::MouseButtonPressed:
		if (currentEvent.mouseButton.button == sf::Mouse::Left)
		{
			onLeftClick(mousePosition);
			handleUIInteraction(mousePosition);
		}
		else if (currentEvent.mouseButton.button == sf::Mouse::Right)
		{
			onRightClick(mousePosition);
		}

		break;

	case sf::Event::MouseMoved:
		onMouseMove(mousePosition);
		mousePosition -= MOUSE_POSITION_OFFSET;
		m_battleUILayer.onComponentIntersect(mousePosition);
		
		break;
	case sf::Event::MouseButtonReleased:
		if (m_leftClickHeld)
		{
			onLeftClickReleased(mousePosition);
		}
		else if(m_rightClickHeld)
		{
			onRightClickReleased(mousePosition);
		}
		
		break;
	}
}

void BattleUI::update(float deltaTime)
{	
	m_battleUILayer.update(deltaTime);

	//Move Camera 'x' axis
	m_cameraPosition.x += m_cameraVelocity.x;
	if (m_cameraPosition.x < m_cameraBounds.left)
	{
		m_cameraPosition.x = m_cameraBounds.left;
	}
	else if (m_cameraPosition.x > m_cameraBounds.width)
	{
		m_cameraPosition.x = m_cameraBounds.width;
	}

	//Move camera 'y' axis
	m_cameraPosition.y += m_cameraVelocity.y;
	if (m_cameraPosition.y < m_cameraBounds.top)
	{
		m_cameraPosition.y = m_cameraBounds.top;
	}
	else if (m_cameraPosition.y > m_cameraBounds.height)
	{
		m_cameraPosition.y = m_cameraBounds.height;
	}
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

	Ray2D startPos = { ship.getCurrentPosition(), ship.getCurrentDirection() };
	PathFinding::getInstance().findArea(m_movementArea.m_tileArea, m_battle.getMap(), startPos, ship.getMovementPoints());

	m_movementArea.activateGraph();
}

void BattleUI::onLeftClickReleased(sf::Vector2i mousePosition)
{
	m_leftClickHeld = false;
	
	if (m_shipSelector.getSelectedShips().empty() && m_tileOnLeftClick && m_tileOnLeftClick->isShipOnTile())
	{
		//Health
		//Attack Range
		//Movement Range
		//Attack Damage
		m_battleUILayer.setComponentVisibility(eUIComponentName::eShipStats, eUIComponentType::eImage, true);
		m_battleUILayer.setComponentVisibility(eUIComponentName::eShipStatHealth, eUIComponentType::eTextBox, true);
		m_battleUILayer.setComponentVisibility(eUIComponentName::eShipStatAttackRange, eUIComponentType::eTextBox, true);
		m_battleUILayer.setComponentVisibility(eUIComponentName::eShipStatMovementRange, eUIComponentType::eTextBox, true);
		m_battleUILayer.setComponentVisibility(eUIComponentName::eShipStatDamage, eUIComponentType::eTextBox, true);

		const Ship& shipOnTile = m_battle.getFactionShip(m_tileOnLeftClick->m_shipOnTile);
		m_battleUILayer.setText(eUIComponentName::eShipStatHealth, std::to_string(shipOnTile.getHealth()));
		m_battleUILayer.setText(eUIComponentName::eShipStatAttackRange, std::to_string(shipOnTile.getRange()));
		m_battleUILayer.setText(eUIComponentName::eShipStatMovementRange, std::to_string(shipOnTile.getMovementPoints()));
		m_battleUILayer.setText(eUIComponentName::eShipStatDamage, std::to_string(shipOnTile.getDamage()));
		
	}
	else if (m_shipSelector.getSelectedShips().size() == 1)
	{
		m_battleUILayer.setComponentVisibility(eUIComponentName::eShipStats, eUIComponentType::eImage, true);

		m_battleUILayer.setComponentVisibility(eUIComponentName::eShipStats, eUIComponentType::eImage, true);
		m_battleUILayer.setComponentVisibility(eUIComponentName::eShipStatHealth, eUIComponentType::eTextBox, true);
		m_battleUILayer.setComponentVisibility(eUIComponentName::eShipStatAttackRange, eUIComponentType::eTextBox, true);
		m_battleUILayer.setComponentVisibility(eUIComponentName::eShipStatMovementRange, eUIComponentType::eTextBox, true);
		m_battleUILayer.setComponentVisibility(eUIComponentName::eShipStatDamage, eUIComponentType::eTextBox, true);

		const Ship& shipOnTile = m_battle.getFactionShip(m_shipSelector.getSelectedShips()[0].m_shipOnTile);
		m_battleUILayer.setText(eUIComponentName::eShipStatHealth, std::to_string(shipOnTile.getHealth()));
		m_battleUILayer.setText(eUIComponentName::eShipStatAttackRange, std::to_string(shipOnTile.getRange()));
		m_battleUILayer.setText(eUIComponentName::eShipStatMovementRange, std::to_string(shipOnTile.getMovementPoints()));
		m_battleUILayer.setText(eUIComponentName::eShipStatDamage, std::to_string(shipOnTile.getDamage()));
	}
	else
	{
		m_battleUILayer.setComponentVisibility(eUIComponentName::eShipStats, eUIComponentType::eImage, false);
		m_battleUILayer.setComponentVisibility(eUIComponentName::eShipStatHealth, eUIComponentType::eTextBox, false);
		m_battleUILayer.setComponentVisibility(eUIComponentName::eShipStatAttackRange, eUIComponentType::eTextBox, false);
		m_battleUILayer.setComponentVisibility(eUIComponentName::eShipStatMovementRange, eUIComponentType::eTextBox, false);
		m_battleUILayer.setComponentVisibility(eUIComponentName::eShipStatDamage, eUIComponentType::eTextBox, false);
	}

	if (!m_battle.isShipBelongToCurrentFaction(m_tileOnLeftClick->m_shipOnTile) ||
		m_battle.getCurrentFaction().m_controllerType != eFactionControllerType::eLocalPlayer)
	{
		return;
	}

	if (m_battle.getCurrentBattlePhase() == eBattlePhase::Movement)
	{
		onLeftClickMovementPhase();
	}
	else if (m_battle.getCurrentBattlePhase() == eBattlePhase::Attack)
	{
		onLeftClickAttackPhase();
	}
}

void BattleUI::onRightClickReleased(sf::Vector2i mousePosition)
{
	m_rightClickHeld = false;
	m_spriteOnMouse.deactivate();

	auto mouseDirection = Math::calculateDirection(m_rightClickPosition, mousePosition);
	switch (m_battle.getCurrentBattlePhase())
	{
	case eBattlePhase::Deployment:
		onRightClickReleasedDeploymentPhase(mouseDirection.second);
		break;

	case eBattlePhase::Movement:
		onRightClickReleasedMovementPhase(mouseDirection, mousePosition);
		break;

	case eBattlePhase::Attack:
		onRightClickReleasedAttackPhase();
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
	m_tileOnMouse = tileOnMouse;
	//First Click
	if (!m_tileOnLeftClick)
	{
		m_tileOnLeftClick = tileOnMouse;
		m_tileOnMouse = tileOnMouse;
	}

	eBattlePhase currentBattlePhase = m_battle.getCurrentBattlePhase();

	//Clicked on new tile
	if (m_tileOnLeftClick->m_tileCoordinate != tileOnMouse->m_tileCoordinate)
	{
		if (currentBattlePhase == eBattlePhase::Movement)
		{
			m_movementArea.clearTileArea();
			if (m_tileOnLeftClick->isShipOnTile())
			{
				m_battle.clearFactionShipMovementArea(m_tileOnLeftClick->m_shipOnTile);
			}
		}
		else if (currentBattlePhase == eBattlePhase::Attack)
		{
			m_targetArea.clearTileArea();
		}

		m_tileOnLeftClick = tileOnMouse;
		m_tileOnMouse = tileOnMouse;
	}


	size_t selectedShipCount = m_shipSelector.getSelectedShips().size();
	if (selectedShipCount > 0)
	{
		for (int i = 0; i < selectedShipCount; ++i)
		{
			ShipOnTile selectedShip = m_shipSelector.removeSelectedShip();
			if (currentBattlePhase == eBattlePhase::Movement)
			{
				m_battle.clearFactionShipMovementArea(selectedShip);
			}
		}
	}
}

void BattleUI::onLeftClickMovementPhase()
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

void BattleUI::onLeftClickAttackPhase()
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

void BattleUI::onRightClickReleasedAttackPhase()
{
	size_t selectedShipCount = m_shipSelector.getSelectedShips().size();
	if (selectedShipCount > 0)
	{
		for (const auto& selectedShip : m_shipSelector.getSelectedShips())
		{
			const Tile* tileOnSelectedShip = m_battle.getMap().getTile(selectedShip.m_sprite.getPosition());
			assert(tileOnSelectedShip);
			generateTargetArea(*tileOnSelectedShip);

			m_battle.fireFactionShipAtPosition(selectedShip.m_shipOnTile, *m_tileOnRightClick, m_targetArea.m_tileArea);
		}

		m_shipSelector.reset();
	}
	else
	{
		if (m_tileOnRightClick->isShipOnTile() && !m_battle.isShipBelongToCurrentFaction(m_tileOnRightClick->m_shipOnTile))
		{
			m_battle.fireFactionShipAtPosition(m_tileOnLeftClick->m_shipOnTile, *m_tileOnRightClick, m_targetArea.m_tileArea);
		}
		else if (m_tileOnLeftClick && m_tileOnLeftClick->isShipOnTile() && !m_tileOnRightClick->isShipOnTile())
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
	if (m_leftClickHeld && m_battle.getCurrentBattlePhase() != eBattlePhase::Deployment)
	{
		m_shipSelector.update(m_battle, mousePosition, m_battle.getMap());
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
		case eBattlePhase::Deployment:
		{
			onMouseMoveDeploymentPhase(mousePosition);
			break;
		}
		case eBattlePhase::Movement:
		{
			onMouseMoveMovementPhase(mousePosition);
			break;
		}
		case eBattlePhase::Attack:
		{
			onMouseMoveAttackPhase();
			break;
		}
		}
	}
}

void BattleUI::moveCamera(sf::Vector2u windowSize, sf::Vector2i mousePosition)
{
 	sf::Vector2i cameraMoveBoundary(windowSize.x / CAMERA_MOVEMENT_BOUNDARY, windowSize.y / CAMERA_MOVEMENT_BOUNDARY);

	if (mousePosition.x < cameraMoveBoundary.x)
	{
		m_cameraVelocity.x = -CAMERA_MOVE_SPEED.x;
	}
	else if (mousePosition.x > windowSize.x - cameraMoveBoundary.x)
	{
		m_cameraVelocity.x = CAMERA_MOVE_SPEED.x;
	}
	else
	{
		m_cameraVelocity.x = 0.0f;
	}

	if (mousePosition.y < cameraMoveBoundary.y)
	{
		m_cameraVelocity.y = -CAMERA_MOVE_SPEED.y;
	}
	else if (mousePosition.y > windowSize.y - cameraMoveBoundary.y)
	{
		m_cameraVelocity.y = CAMERA_MOVE_SPEED.y;
	}
	else
	{
		m_cameraVelocity.y = 0.0f;
	}
}

void BattleUI::onMouseMoveDeploymentPhase(sf::Vector2i mousePosition)
{
	if (!m_battle.getCurrentFaction().isPositionInDeploymentArea(m_tileOnMouse->m_tileCoordinate))
	{
		m_spriteOnMouse.activate();
		m_spriteOnMouse.setPosition(m_tileOnMouse->m_tileCoordinate);
	}
	else if (m_rightClickHeld && m_battle.getCurrentFaction().isPositionInDeploymentArea(m_tileOnMouse->m_tileCoordinate))
	{
		m_spriteOnMouse.setPosition(m_tileOnMouse->m_tileCoordinate);
		m_spriteOnMouse.activate();
	}
	else if (m_rightClickHeld && !m_battle.getCurrentFaction().isPositionInDeploymentArea(m_tileOnMouse->m_tileCoordinate))
	{
		m_spriteOnMouse.deactivate();
	}
	else if (m_battle.getCurrentFaction().isPositionInDeploymentArea(m_tileOnMouse->m_tileCoordinate))
	{
		m_spriteOnMouse.deactivate();
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
	if (m_rightClickHeld)
	{
		m_spriteOnMouse.setPosition(m_tileOnMouse->m_tileCoordinate);
		return;
	}

	//Multiple ships selected
	if (m_shipSelector.getSelectedShips().size() > size_t(1) && !m_leftClickHeld)
	{
		m_movementArea.clearTileArea();
		m_battle.getMap().getTileRadius(m_movementArea.m_tileArea, m_tileOnMouse->m_tileCoordinate, 
			MINIMUM_MULTIPLE_SHIP_SEARCH_AREA, true, false);

		if (m_movementArea.m_tileArea.empty())
		{
			return;
		}

		int range = MINIMUM_MULTIPLE_SHIP_SEARCH_AREA + 1;
		while (m_movementArea.m_tileArea.size() < m_shipSelector.getSelectedShips().size())
		{
			m_battle.getMap().getTileRadius(m_movementArea.m_tileArea, m_tileOnMouse->m_tileCoordinate, range, true, true);

			++range;
		}

		m_battle.generateFactionShipsMovementArea(m_movementArea.m_tileArea, m_shipSelector);
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

void BattleUI::onRightClickReleasedMovementPhase(std::pair<double, eDirection> mouseDirection, sf::Vector2i mousePosition)
{	
	if (!m_shipSelector.getSelectedShips().empty())
	{
		m_battle.moveFactionShipsToPosition(m_shipSelector);
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
		}
	}

	m_movementArea.clearTileArea();
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
		m_tileOnMouse = tileOnMouse;
		if (m_battle.getCurrentBattlePhase() != eBattlePhase::Deployment)
		{
			m_spriteOnMouse.activate();
			m_spriteOnMouse.setPosition(m_tileOnMouse->m_tileCoordinate);
		}
	}
	else
	{
		m_rightClickHeld = false;
		m_spriteOnMouse.deactivate();
	}
}

void BattleUI::onRightClickReleasedDeploymentPhase(eDirection startingDirection)
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

void BattleUI::onCancelMovementPhase()
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

void BattleUI::onCancelAttackPhase()
{
	m_tileOnLeftClick = nullptr;
	m_spriteOnTileClick.deactivate();
	m_targetArea.clearTileArea();
}

void BattleUI::onMouseMoveAttackPhase()
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

void BattleUI::handleUIInteraction(sf::Vector2i mousePosition)
{
	mousePosition -= MOUSE_POSITION_OFFSET;
	UIComponentIntersectionDetails intersectionDetails;
	m_battleUILayer.onComponentIntersect(mousePosition, intersectionDetails);
	if (intersectionDetails.isIntersected())
	{
		switch (intersectionDetails.getComponentName())
		{
		case eUIComponentName::eEndPhase:
			m_battle.endCurrentBattlePhase();
			if (NetworkHandler::getInstance().isConnectedToServer())
			{
				NetworkHandler::getInstance().sendMessageToServer({ eMessageType::ePlayerEndedPhase, m_battle.getLocalControlledFaction() });
			}

			break;
		case eUIComponentName::ePause:
			GameEventMessenger::getInstance().broadcast(GameEvent(), eGameEvent::eQuitGame);

			break;
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