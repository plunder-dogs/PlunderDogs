#pragma once

#include "Global.h"
#include "Battle.h"
#include "OverWorldGUI.h"
#include "entity.h"

using namespace HAPI_UI_SPACE;
using namespace HAPISPACE;

struct ShipProperties
{
	ShipProperties(FactionName factionName, ShipType entityType);
	ShipProperties() {};

	std::shared_ptr<HAPISPACE::Sprite> m_sprite;
	std::shared_ptr<HAPISPACE::Sprite> m_selectedSprite;
	int m_movementPoints;
	int m_originalMovement;
	int m_healthMax;
	int m_originalHealth;
	int m_currentHealth;
	int m_range;
	int m_originalRange;
	int m_damage;
	int m_originalDamage;
	int m_weaponType;
	int m_upgradePoints;
	int m_maxUpgradePoints;
};

struct PlayerDetails
{
	std::vector<ShipProperties> m_selectedEntities;

	FactionName m_factionName;
	ePlayerType m_type;
};

class OverWorld : public IHapiSpritesInputListener
{
public:
	OverWorld();
	OverWorld(const OverWorld&) = delete;
	OverWorld& operator=(const OverWorld&) = delete;
	OverWorld(OverWorld&&) = delete;
	OverWorld&& operator=(OverWorld&&) = delete;
	~OverWorld();

	void OnMouseEvent(EMouseEvent mouseEvent, const HAPI_TMouseData& mouseData) override final;
	void OnMouseMove(const HAPI_TMouseData& mouseData) override final;
	void OnKeyEvent(EKeyEvent keyEvent, BYTE keyCode) override final {}

	void render() const;
	void update(float deltaTime);
	void startBattle();
private:
	int m_currentPlayer;
	bool m_selectNextPlayer;
	std::vector<Player> m_players;
	OverWorldGUI m_GUI;
	Battle m_battle;
	bool m_startBattle;

	void onReset();
};