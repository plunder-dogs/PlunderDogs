#pragma once

#include <HAPISprites_lib.h>
#include <HAPISprites_UI.h>
#include <memory>

#include "Global.h"
#include <string>

struct Player;
struct EntityProperties;
class OverWorld;
class Battle;
class OverWorldGUI 
{
private:
	EntityProperties* m_currentlySelected;
	bool m_enitiesAdded;

	//m_enemyTerritoryHexSheet   might need name for replacing
	std::unique_ptr<Sprite> m_battleMapBackground;
	std::unique_ptr<Sprite> m_selectMapButtons1;
	std::unique_ptr<Sprite> m_selectMapButtons2;
	std::unique_ptr<Sprite> m_selectMapButtons3;
	std::unique_ptr<Sprite> m_selectMapButtons4;
	std::unique_ptr<Sprite> m_selectMapButtons5;

	std::unique_ptr<Sprite> m_playerSelectYellow;
	std::unique_ptr<Sprite> m_playerSelectGreen;
	std::unique_ptr<Sprite> m_playerSelectRed;
	std::unique_ptr<Sprite> m_playerSelectBlue;
	std::unique_ptr<Sprite> m_playerSelectBackground;
	std::unique_ptr<Sprite> m_prebattleUIBackground;
	std::unique_ptr<Sprite> m_playButton;
	std::unique_ptr<Sprite> m_backButton;
	std::unique_ptr<Sprite> m_done;

	std::unique_ptr<Sprite> m_background;
	std::unique_ptr<Sprite> m_quitButton;

	void positionEntity(const std::string& windowName, const std::string& windowSliderName, const std::string& windowObjectName, int objectNumber, size_t vectorSize);
	void positionUpgradeEntity(const std::string& windowName, const std::string& windowScrollbarName, const std::string& windowObjectName, int objectNumber, size_t vectorSize);
	float getWindowSliderValue(const std::string &windowName, const std::string &windowSliderName) const;
	HAPISPACE::RectangleI calculateObjectWindowPosition(int objectNumber) const;
	HAPISPACE::RectangleI calculateUpgradeObjectWindowPosition(int objectNumber) const;
	HAPISPACE::RectangleI windowScreenRect(const std::string& windowName) const;
	HAPISPACE::VectorI calculateObjectScrolledPosition(const std::string& windowName, const std::string& windowSliderName, int objectNumber, size_t vectorSize);
	HAPISPACE::VectorI calculateUpgradeObjectScrolledPosition(const std::string& windowName, const std::string& windowSliderName, int objectNumber, size_t vectorSize);
	bool entityContainsMouse(const std::string& windowName, const std::string& windowObjectName, HAPISPACE::VectorI windowTopLeft, HAPISPACE::VectorI mousePosition) const;
	bool windowObjectExists(const std::string& windowName, const std::string& windowObjectName) const;
	void checkShipSelect(bool& selection, const std::string& shipWindow, const std::string& windowSlider, const HAPISPACE::VectorI& mouseData, const HAPISPACE::VectorI& windowTopLeft, std::vector<EntityProperties>& entities, const bool vertical = false);
	void checkShipSelect(bool& selection, const std::string& shipWindow, const std::string& windowSlider, const HAPISPACE::VectorI& mouseData, const HAPISPACE::VectorI& windowTopLeft, std::vector<EntityProperties*>& entities, const bool vertical = false);
	void selectBattleShip(const std::string& shipWindow, const std::string& windowSlider, const std::string& selectedShipWindow, const std::string& selectedWindowSlider, const HAPISPACE::VectorI& mouseData, const HAPISPACE::VectorI& windowTopLeft, const HAPISPACE::VectorI& selectedTopLeft, std::vector<EntityProperties>& entities, std::vector<EntityProperties*>& selectedEntities);
	void deselectBattleShip(const std::string& selectedShipWindow, const std::string& selectedShipSlider, const HAPISPACE::VectorI& selectedWindowTopLeft, std::vector<EntityProperties*>& selectedEntities, const HAPISPACE::VectorI& mouseData);
	void updateSelectedShips(const std::string& shipWindow, const HAPISPACE::VectorI& windowTopLeft, std::vector<EntityProperties>& entities, std::vector<EntityProperties*>& selectedEntities);
	void onReset();

	HAPISPACE::SliderLayout sliderLayout;
	HAPISPACE::VectorI m_fleetWindowTopLeft{ HAPISPACE::VectorI(350, 600) };
	HAPISPACE::VectorI m_battleFleetWindowTopLeft{ HAPISPACE::VectorI(350, 300) };
	HAPISPACE::VectorI m_upgradeFleetWindowTopLeft{ HAPISPACE::VectorI(380,210) };

	int m_currentShips{ 0 };

	const std::string fleetWindowSliderSkinName;
	const std::string fleetWindowSkinName;
	const std::string ENTITY = "entity";
	const std::string FLEET_WINDOW = "fleetwindow";
	const std::string BATTLE_FLEET_WINDOW = "battleFleetWindow";
	const std::string FLEET_SLIDER = "fleetSlider";
	const std::string BATTLE_FLEET_SLIDER = "battleFleetSlider";
	const std::string UPGRADE_FLEET_WINDOW = "upgradefleetwindow";
	const std::string UPGRADE_FLEET_SCROLLBAR = "upgradefleetscrollbar";


	std::unique_ptr<Sprite> m_upgradesButton;
	//upgrade buttons
	std::unique_ptr<Sprite> m_upgradesScreenBackground;
	std::unique_ptr<Sprite> m_removeHealthButton;
	std::unique_ptr<Sprite> m_removeMovementButton;
	std::unique_ptr<Sprite> m_removeDamageButton;
	std::unique_ptr<Sprite> m_removeRangeButton;
	std::unique_ptr<Sprite> m_addHealthButton;
	std::unique_ptr<Sprite> m_addMovementButton;
	std::unique_ptr<Sprite> m_addDamageButton;
	std::unique_ptr<Sprite> m_addRangeButton;
	std::unique_ptr<Sprite> m_upgradeBackButton;
	std::string selectedMap = "Level1.tmx";
	std::string selectedMapName = "Isle of Turtles";
	bool shipSelectionTrigger{ false };
	int getActivePlayerCount();
	bool leftPlayerSelectionTrig{ false };
	
	

public:

	void setActivePlayers(std::vector<Player>&players);
	void setShipSelectionTrigger(bool trigger);
	std::string getSelectedMap();
	static OverWorldWindow CURRENT_WINDOW;
	OverWorldGUI();
	~OverWorldGUI();
	bool getLeftPlayerSelectionTrig();
	void setLeftPlayerSelectionTrig(bool trigger);
	void onLeftClick(const HAPI_TMouseData& mouseData, Player& currentSelectedPlayer, bool& selectNextPlayer,bool& resetPlayer);
	//void onRightClick(const HAPI_TMouseData& mouseData, Player& currentSelectedPlayer);
	void onMouseMove(const HAPI_TMouseData& mouseData, Player& currentSelectedPlayer);

	void reset(const std::vector<EntityProperties>& playerEntities);
	void clear();

	void render(const Battle& battle) const;
};