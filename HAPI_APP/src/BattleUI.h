#pragma once

#include "Global.h"
#include "BattleGUI.h"
#include "TileArea.h"

struct Tile;
class Ship;
class Battle;
class Map;
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

	void renderUI(sf::RenderWindow& window);
	void renderGUI(sf::RenderWindow& window);
	void loadGUI(sf::Vector2i mapDimensions);

	void drawTargetArea(sf::RenderWindow& window);
	void handleInput(sf::RenderWindow& window, const sf::Event& currentEvent);
	void update(float deltaTime);

private:
	Battle& m_battle;
	const Tile* m_tileOnPreviousClick;
	const Tile* m_tileOnClick;
	const Tile* m_tileOnMouse;
	Sprite m_tileHighlight;
	BattleGUI m_gui;
	TileArea m_movementArea;
	TileArea m_targetArea;

	void onNewBattlePhase();
	void generateTargetArea(const Tile& source);
	void generateMovementArea(const Ship& ship);
	void renderTileHighlight(sf::RenderWindow& window);

	//LeftClick
	void onLeftClick(sf::RenderWindow& window);
	void onLeftClickDeploymentPhase(eDirection startingDirection = eDirection::eNorth);
	void onLeftClickMovementPhase(sf::Vector2i mousePosition);
	void onLeftClickAttackPhase(sf::Vector2i mousePosition);
	//MouseMove
	void handleMouseMovement(sf::Vector2i mousePosition);
	void onMouseMoveDeploymentPhase(sf::Vector2i mousePosition);
	void onMouseMoveMovementPhase(sf::Vector2i mousePosition);
	void onMouseMoveAttackPhase(sf::Vector2i mousePosition);
	//RightClick
	void onRightClickMovementPhase(sf::Vector2i mousePosition);
	void onRightClickAttackPhase(sf::Vector2i mousePosition);
};