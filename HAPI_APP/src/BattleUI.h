#pragma once

#include <vector>
#include <SFML/Graphics.hpp>
#include "Global.h"
#include "BattleGUI.h"
#include <memory>

class Ship;
struct Tile;
class Battle;
class Map;
class BattleUI
{
	//For displaying and remembering the location of the movement area of a selected ship
	struct MovementArea
	{
		MovementArea(std::unique_ptr<sf::Texture>& texturePtr, const Map& map);
		MovementArea(const MovementArea&) = delete;
		MovementArea& operator=(const MovementArea&) = delete;
		MovementArea(MovementArea&&) = delete;
		MovementArea&& operator=(MovementArea&&) = delete;

		void render(sf::RenderWindow& window, const Map& map);
		void newArea(const Map& map, const Ship& ship);
		void clear();

		bool m_display;
		int m_displaySize;
		std::vector<std::pair<sf::Vector2i, sf::Sprite>> m_tileOverlays;
	};

	struct TargetArea
	{
		struct HighlightNode
		{
			HighlightNode();
			sf::Sprite sprite;
			bool activate;
			sf::Vector2i position;
		};

		TargetArea();
		void render(sf::RenderWindow& window, const Map& map);
		void generateTargetArea(Battle& battle, const Tile& source, BattlePhase phase = BattlePhase::Attack);
		void clearTargetArea();
		void onReset();

		std::vector<HighlightNode> m_targetAreaSprites;
		std::vector<const Tile*> m_targetArea;
	};

	struct InvalidPosition
	{
		InvalidPosition();

		void render(sf::RenderWindow& window, const Map& map);
		void setPosition(sf::Vector2i newPosition, const Map& map);
		void onReset();

		sf::Sprite m_sprite;
		bool m_activate;
		sf::Vector2i m_position;
	};

	struct SelectedTile
	{
		SelectedTile();

		void render(sf::RenderWindow& window, const Map& map);

		sf::Sprite m_sprite;
		const Tile* m_tile;
		sf::Vector2i m_position;
	};

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
	void onFactionWin(FactionName winningFaction);
	void onEnteringBattlePhase(BattlePhase currentBattlePhase);

	void drawTargetArea(sf::RenderWindow& window);

	void handleInput(sf::RenderWindow& window, const sf::Event& currentEvent);
	void update(float deltaTime);

	void clearSelectedTile();
	void FactionUpdateGUI(FactionName faction);
	void setCurrentFaction(FactionName faction);
	bool isPaused();

private:
	Battle& m_battle;
	SelectedTile m_selectedTile;
	//This stores the position at which the mouse event "eLeftMouseButtonDown" last occured while giving an entity a move command, 
	//it's used to calculate what direction the mouse moved during that input.
	sf::Vector2i m_leftMouseDownPosition;
	//This is used to determine if an entity is currently being given a move command, it gets set to true in the "handleOnLeftClickMovementPhase()" and false after "eLeftMouseButtonUp" is detected.
	bool m_isMovingEntity;
	//Used to store the tile selected for movement when the lmb is depressed, so that it can be used for moveEntity input on mouse up
	const Tile* m_mouseDownTile;
	InvalidPosition m_invalidPosition;
	//std::deque<std::unique_ptr<DeploymentPhase>> m_shipDeployment;
	//Directional arrow
	sf::Vector2i m_lastMouseData;
	bool m_arrowActive;
	sf::Sprite m_arrowSprite;
	BattleGUI m_gui;

	//void renderArrow() const;

	//Deployment Phase
	void onMouseMoveDeploymentPhase(sf::Vector2i mousePosition);
	void onLeftClickDeploymentPhase(eDirection startingDirection = eDirection::eNorth);

	//Movement Phase
	void onMouseMoveMovementPhase(sf::Vector2i mousePosition);
	void onLeftClickMovementPhase(sf::Vector2i mousePosition);
	void onRightClickMovementPhase(sf::Vector2i mousePosition);
	MovementArea m_movementArea;

	//Attack Phase
	void onLeftClickAttackPhase(sf::Vector2i mousePosition);
	void onRightClickAttackPhase(sf::Vector2i mousePosition);
	void onMouseMoveAttackPhase(sf::Vector2i mousePosition);
	TargetArea m_targetArea;

	void onResetBattle();
	void onNewTurn();
	void clearTargetArea();
};