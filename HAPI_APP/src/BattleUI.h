#pragma once

#include "Global.h"
#include "BattleGUI.h"
#include "TileArea.h"

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
	void loadGUI(sf::Vector2i mapDimensions);

	void handleInput(const sf::RenderWindow& window, const sf::Event& currentEvent);
	void update(float deltaTime);

private:
	Battle& m_battle;
	const Tile* m_tileOnPreviousClick;
	const Tile* m_tileOnClick;
	const Tile* m_tileOnMouse;
	Sprite m_tileOnClickSprite;
	Sprite m_spriteOnMouse;
	BattleGUI m_gui;
	TileArea m_shipMovementArea;
	TileArea m_shipTargetArea;
	
	bool m_leftClickHeld = false;
	sf::Vector2i m_leftClickPosition;

	void onNewBattlePhase(GameEvent gameEvent);
	void generateTargetArea(const Tile& source);
	void generateMovementArea(const Ship& ship);

	//ClickReleased
	void onClickReleased(sf::Vector2i mousePosition);

	//LeftClick
	void onLeftClick(sf::Vector2i mousePosition);
	void onLeftClickDeploymentPhase(eDirection startingDirection = eDirection::eNorth);
	void onLeftClickMovementPhase(sf::Vector2i mousePosition);
	void onLeftClickAttackPhase(sf::Vector2i mousePosition);
	//MouseMove
	void onMouseMovement(sf::Vector2i mousePosition);
	void onMouseMoveDeploymentPhase(sf::Vector2i mousePosition);
	void onMouseMoveMovementPhase(sf::Vector2i mousePosition);
	void onMouseMoveAttackPhase(sf::Vector2i mousePosition);
	//RightClick
	void onRightClick(sf::Vector2i mousePosition);
	void onRightClickMovementPhase(sf::Vector2i mousePosition);
	void onRightClickAttackPhase(sf::Vector2i mousePosition);
};