#pragma once

#include "ShipOnTile.h"
#include "TileArea.h"
#include "Selector.h"
#include <array>

struct Tile;
class Ship;
class Battle;
struct GameEvent;
class BattleUI
{
public:
	BattleUI(Battle& battles);
	BattleUI(const BattleUI&) = delete;
	BattleUI& operator=(const BattleUI&) = delete;
	BattleUI(BattleUI&&) = delete;
	BattleUI&& operator=(BattleUI&&) = delete;
	~BattleUI();

	sf::Vector2i getCameraPositionOffset() const;

	void render(sf::RenderWindow& window);
	void setMaxCameraOffset(sf::Vector2i maxCameraOffset);
	void handleInput(const sf::RenderWindow& window, const sf::Event& currentEvent);
	void update(float deltaTime);

private:
	Battle& m_battle;
	const Tile* m_tileOnPreviousLeftClick;
	const Tile* m_tileOnLeftClick;
	const Tile* m_tileOnRightClick;
	const Tile* m_tileOnMouse;
	Sprite m_spriteOnTileClick;
	Sprite m_spriteOnMouse;
	TileArea m_movementArea;
	TileArea m_targetArea;
	
	bool m_leftClickHeld;
	sf::Vector2i m_leftClickPosition;
	bool m_rightClickHeld = false;
	sf::Vector2i m_rightClickPosition;

	//Camera
	sf::Vector2i m_maxCameraOffset;
	sf::Vector2f m_pendingCameraMovement;
	sf::Vector2i m_cameraPositionOffset;
	void updateCamera();
	
	Selector m_shipSelector;

	void onNewBattlePhase(GameEvent gameEvent);
	void generateTargetArea(const Tile& source);
	void generateMovementArea(const Ship& ship);
	
	//KeyPress
	void onKeyPress(sf::Vector2i mousePosition, const sf::Event& currentEvent);

	//ClickReleased
	void onLeftClickReleased(sf::Vector2i mousePosition);
	void onRightClickReleased(sf::Vector2i mousePosition);

	//LeftClick
	void onLeftClick(sf::Vector2i mousePosition);
	void onLeftClickMovementPhase(std::pair<double, eDirection> mouseDirection, sf::Vector2i mousePosition);
	void onLeftClickAttackPhase(sf::Vector2i mousePosition);
	//MouseMove
	void onMouseMove(sf::Vector2i mousePosition);
	void moveCamera(sf::Vector2i mousePosition);
	void onMouseMoveDeploymentPhase(sf::Vector2i mousePosition);
	void onMouseMoveMovementPhase(sf::Vector2i mousePosition);
	void onMouseMoveAttackPhase(sf::Vector2i mousePosition);
	void handleOnMouseMoveShipSelector();
	//RightClick
	void onRightClick(sf::Vector2i mousePosition);
	void onRightClickDeploymentPhase(eDirection startingDirection = eDirection::eNorth);
	void onCancelMovementPhase(sf::Vector2i mousePosition);
	void onCancelAttackPhase(sf::Vector2i mousePosition);
};