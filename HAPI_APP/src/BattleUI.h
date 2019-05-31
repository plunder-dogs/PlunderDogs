#pragma once

#include "BattleGUI.h"
#include "entity.h"
#include <vector>
#include <deque>


constexpr int MAX_MOVE_AREA{ 700 };
struct Tile;
class Battle;
class Map;
class BattleUI : public IHapiSpritesInputListener
{
	//For displaying and remembering the location of the movement area of a selected ship
	struct MovementArea
	{
		bool m_display;
		int m_displaySize;
		std::vector<std::pair<std::pair<int, int>, std::unique_ptr<HAPISPACE::Sprite>>> m_tileOverlays;

		MovementArea(std::shared_ptr<HAPISPACE::SpriteSheet> texturePtr) : m_display(false), m_displaySize(0), m_tileOverlays()
		{
			m_tileOverlays.resize(MAX_MOVE_AREA);
			for (int i = 0; i < MAX_MOVE_AREA; i++)
			{
				m_tileOverlays[i].second = std::make_unique<HAPISPACE::Sprite>(texturePtr);
				m_tileOverlays[i].second->GetTransformComp().SetOriginToCentreOfFrame();
			}
		}
		void render(const Battle& battle) const;
		void newArea(const Battle& battle, BattleEntity& ship);
		void clear()
		{
			m_display = false;
			m_displaySize = 0;
		}
	};

	struct TargetArea
	{
		struct HighlightNode
		{
			HighlightNode();
			std::unique_ptr<Sprite> sprite;
			bool activate;
			std::pair<int, int> position;
		};

		TargetArea();
		void render(const Map& map) const;
		void generateTargetArea(Battle& battle, const Tile& source, BattlePhase phase = BattlePhase::Attack);
		void clearTargetArea();
		void onReset();

		std::vector<HighlightNode> m_targetAreaSprites;
		std::vector<const Tile*> m_targetArea;
	};

	struct InvalidPosition
	{
		InvalidPosition();

		void render(const Map& map) const;
		void setPosition(std::pair<int, int> newPosition, const Map& map);
		void onReset();

		std::unique_ptr<Sprite> m_sprite;
		bool m_activate;
		std::pair<int, int> m_position;
	};

	struct SelectedTile
	{
		SelectedTile();

		void render(const Map& map) const;

		std::unique_ptr<Sprite> m_sprite;
		const Tile* m_tile;
		std::pair<int, int> m_position;
	};

public:
	BattleUI(Battle& battles);
	BattleUI(const BattleUI&) = delete;
	BattleUI& operator=(const BattleUI&) = delete;
	BattleUI(BattleUI&&) = delete;
	BattleUI&& operator=(BattleUI&&) = delete;
	~BattleUI();

	std::pair<int, int> getCameraPositionOffset() const;

	void renderUI() const;
	void renderGUI() const;
	void loadGUI(std::pair<int, int> mapDimensions);
	void onFactionWin(FactionName winningFaction);
	void onEnteringBattlePhase(BattlePhase currentBattlePhase);

	void drawTargetArea() const;

	void update(float deltaTime);

	void clearSelectedTile();
	void FactionUpdateGUI(FactionName faction);

	void OnKeyEvent(EKeyEvent keyEvent, BYTE keyCode) override final {}
	void OnMouseEvent(EMouseEvent mouseEvent, const HAPI_TMouseData& mouseData) override final;
	void OnMouseMove(const HAPI_TMouseData& mouseData) override final;
	void setCurrentFaction(FactionName faction);
	bool isPaused();

private:
	Battle& m_battle;
	SelectedTile m_selectedTile;
	//This stores the position at which the mouse event "eLeftMouseButtonDown" last occured while giving an entity a move command, 
	//it's used to calculate what direction the mouse moved during that input.
	std::pair<int, int> m_leftMouseDownPosition;
	//This is used to determine if an entity is currently being given a move command, it gets set to true in the "handleOnLeftClickMovementPhase()" and false after "eLeftMouseButtonUp" is detected.
	bool m_isMovingEntity;
	//Used to store the tile selected for movement when the lmb is depressed, so that it can be used for moveEntity input on mouse up
	const Tile* m_mouseDownTile;
	BattleGUI m_gui;
	InvalidPosition m_invalidPosition;
	//std::deque<std::unique_ptr<DeploymentPhase>> m_shipDeployment;
	//Directional arrow
	std::pair<int, int> m_lastMouseData;
	bool m_arrowActive;
	std::unique_ptr<Sprite> m_arrowSprite;

	void renderArrow() const;

	//Deployment Phase
	void onMouseMoveDeploymentPhase();
	void onLeftClickDeploymentPhase(eDirection startingDirection = eDirection::eNorth);

	//Movement Phase
	void onMouseMoveMovementPhase();
	void onLeftClickMovementPhase();
	void onRightClickMovementPhase();
	MovementArea m_movementArea;

	//Attack Phase
	void onLeftClickAttackPhase();
	void onRightClickAttackPhase();
	void onMouseMoveAttackPhase();
	TargetArea m_targetArea;

	void onResetBattle();
	void onNewTurn();
	void clearTargetArea();
};