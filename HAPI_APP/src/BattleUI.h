#pragma once

#include "TileArea.h"
#include "ShipSelector.h"
#include "Utilities/NonCopyable.h"

struct Tile;
class Ship;
class Battle;
struct GameEvent;
class BattleUI : private NonCopyable
{
public:
	BattleUI(Battle& battle);
	~BattleUI();

	TileArea& getTargetArea();
	sf::Vector2i getCameraPosition() const;

	void moveCamera(sf::Vector2u windowSize, sf::Vector2i mousePosition);
	void render(sf::RenderWindow& window);
	void setCameraBounds(sf::Vector2i mapDimensions);
	void handleInput(const sf::Event& currentEvent, sf::Vector2i mousePosition);
	void update(float deltaTime);

private:
	Battle& m_battle;
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
	sf::IntRect m_cameraBounds;
	sf::Vector2f m_cameraVelocity;
	sf::Vector2i m_cameraPosition;
	
	ShipSelector m_shipSelector;

	void onNewBattlePhase(GameEvent gameEvent);
	void generateTargetArea(const Tile& source);
	void generateMovementArea(const Ship& ship);

	//KeyPress
	void onCancelMovementPhase();
	void onCancelAttackPhase();

	//ClickReleased
	void onLeftClickReleased(sf::Vector2i mousePosition);
	void onRightClickReleased(sf::Vector2i mousePosition);

	//LeftClick
	void onLeftClick(sf::Vector2i mousePosition);
	void onLeftClickMovementPhase();
	void onLeftClickAttackPhase();

	//MouseMove
	void onMouseMove(sf::Vector2i mousePosition);
	void onMouseMoveDeploymentPhase(sf::Vector2i mousePosition);
	void onMouseMoveMovementPhase(sf::Vector2i mousePosition);
	void onMouseMoveAttackPhase();
	
	//RightClick
	void onRightClick(sf::Vector2i mousePosition);
	void onRightClickReleasedDeploymentPhase(eDirection startingDirection = eDirection::eNorth);
	void onRightClickReleasedMovementPhase(std::pair<double, eDirection> mouseDirection, sf::Vector2i mousePosition);
	void onRightClickReleasedAttackPhase();
};