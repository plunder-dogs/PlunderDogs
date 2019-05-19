#pragma once

#include <HAPISprites_lib.h>
#include <HAPISprites_UI.h>
#include "Global.h"
#include "Textures.h"
#include "entity.h"

using namespace HAPI_UI_SPACE;
using namespace HAPISPACE;


enum BattleWindow
{
	eCombat = 0,
	ePause,
	ePostBattle
};

class BattleGUI
{
public:
	BattleGUI();
	~BattleGUI();

	std::pair<int, int> getCameraPositionOffset() const;

	void render(BattlePhase currentBattlePhase) const;
	void renderStats(EntityProperties& entityProperties) const;
	void update(eDirection windDirection);
	void updateFactionToken(int factionName);
	void OnMouseLeftClick(const HAPI_TMouseData& mouseData, BattlePhase currentBattlePhase);
	//void OnMouseScroll could be added
	void OnMouseMove(const HAPI_TMouseData& mouseData, BattlePhase currentBattlePhase);
	void setMaxCameraOffset(std::pair<int, int> maxCameraOffset);
	void snapCameraToPosition(std::pair<int, int> snapLocation);
	bool isPaused();
private:
	BattleWindow m_currentBattleWindow;
	std::pair<int, int> m_maxCameraOffset;
	std::unique_ptr<Sprite> m_battleIcons;
	std::unique_ptr<Sprite> m_pauseButton;
	std::unique_ptr<Sprite> m_chickenButton;
	std::unique_ptr<Sprite> m_pauseMenuBackground;
	std::unique_ptr<Sprite> m_resumeButton;
	std::unique_ptr<Sprite> m_quitButton;
	std::unique_ptr<Sprite> m_postBattleBackground;
	std::unique_ptr<Sprite> m_doneButton;
	std::unique_ptr<Sprite> m_activeFactionToken;
	std::unique_ptr<Sprite> m_CompassPointer;
	std::unique_ptr<Sprite> m_CompassBackGround;
	std::unique_ptr<Sprite> m_endPhaseButtons;

	DWORD animationStartTime;
	bool shipSelected;
	bool playAnimation = false;
	int animationOffset = 100;
	bool m_AIInPlay;
	
	bool victory = false;
	std::string winningFaction{ "" };
	VectorF pendingCameraMovement{ 0 };
	std::pair<int, int> CameraPositionOffset;//camera offset that will be used by the map
	float cameraZoom = 1.0f;//variable to multiply scale by
	std::pair<int, int> m_cameraPositionOffset;
	
	void onBattleReset();
	std::string getWinningFactionName();
	void onBlueWin();
	void onGreenWin();
	void onYellowWin();
	void onRedWin();
	void onEnteringMovementPhase();
	void onEnteringAttackPhase();
	void onUnableToSkipPhase();
	void onEnteredAITurn();
	void onLeftAITurn();
};