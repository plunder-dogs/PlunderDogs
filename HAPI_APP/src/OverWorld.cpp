#include  "Overworld.h"
#include "Textures.h"
#include "GameEventMessenger.h"
#include "AI.h"

constexpr int UPGRADE_POINTS = 2;
//ENTITY
ShipProperties::ShipProperties(FactionName factionName, ShipType entityType) 
	: 
	m_upgradePoints(UPGRADE_POINTS), 
	m_maxUpgradePoints(UPGRADE_POINTS), 
	m_selectedSprite(HAPI_Sprites.MakeSprite(Textures::m_thing))
{
	//TODO: Currently not working as intended
	//UI seems to be resetting the frameNumber somewhere in OverWorldGUI. 
	switch (entityType)
	{
	case ShipType::eFrigate:
		m_originalHealth = 12;
		m_originalDamage = 4;
		m_originalMovement = 8;
		m_originalRange = 4;

		m_movementPoints = m_originalMovement;
		m_healthMax = m_originalHealth;
		m_currentHealth = m_healthMax;
		m_range = m_originalRange;
		m_damage = m_originalDamage;
		m_weaponType = eWeaponType::eSideCannons;
		break;
	case ShipType::eTurtle:
		m_originalHealth = 20;
		m_originalDamage = 2;
		m_originalMovement = 8;
		m_originalRange = 1;

		m_movementPoints = m_originalMovement;
		m_healthMax = m_originalHealth;
		m_currentHealth = m_healthMax;
		m_range = m_originalRange;
		m_damage = m_originalDamage;
		m_weaponType = eWeaponType::eShotgun;
		break;
	case ShipType::eFire:
		m_originalHealth = 8;
		m_originalDamage = 6;
		m_originalMovement = 10;
		m_originalRange = 2;

		m_movementPoints = m_originalMovement;
		m_healthMax = m_originalHealth;
		m_currentHealth = m_healthMax;
		m_range = m_originalRange;
		m_damage = m_originalDamage;
		m_weaponType = eWeaponType::eFlamethrower;
		break;
	case ShipType::eSniper:
		m_originalHealth = 8;
		m_originalDamage = 4;
		m_originalMovement = 6;
		m_originalRange = 10;

		m_movementPoints = m_originalMovement;
		m_healthMax = m_originalHealth;
		m_currentHealth = m_healthMax;
		m_range = m_originalRange;
		m_damage = m_originalDamage;
		m_weaponType = eWeaponType::eStraightShot;
		break;
	}

	switch (factionName)
	{
	case FactionName::eYellow:
		switch (entityType)
		{
		case ShipType::eFrigate:
			m_sprite = std::shared_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_yellowShipSideCannons));
			break;
		case ShipType::eTurtle:

			m_sprite = std::shared_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_yellowShipBomb));
			break;
		case ShipType::eFire:

			m_sprite = std::shared_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_yellowShipMelee));
			break;
		case ShipType::eSniper:

			m_sprite = std::shared_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_yellowShipSnipe));
			break;
		}

		break;

	case FactionName::eBlue:
		switch (entityType)
		{
		case ShipType::eFrigate:

			m_sprite = std::shared_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_blueShipSideCannons));
			break;
		case ShipType::eTurtle:

			m_sprite = std::shared_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_blueShipBomb));
			break;
		case ShipType::eFire:

			m_sprite = std::shared_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_blueShipMelee));
			break;
		case ShipType::eSniper:

			m_sprite = std::shared_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_blueShipSnipe));
			break;
		}

		break;
	case FactionName::eRed:
		switch (entityType)
		{
		case ShipType::eFrigate:

			m_sprite = std::shared_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_redShipSideCannons));
			break;
		case ShipType::eTurtle:

			m_sprite = std::shared_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_redShipBomb));
			break;
		case ShipType::eFire:

			m_sprite = std::shared_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_redShipMelee));
			break;
		case ShipType::eSniper:

			m_sprite = std::shared_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_redShipSnipe));
			break;
		default:
			break;
		}

		break;
	case FactionName::eGreen:
		switch (entityType)
		{
		case ShipType::eFrigate:

			m_sprite = std::shared_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_greenShipSideCannons));
			break;
		case ShipType::eTurtle:

			m_sprite = std::shared_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_greenShipBomb));
			break;
		case ShipType::eFire:

			m_sprite = std::shared_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_greenShipMelee));
			break;
		case ShipType::eSniper:

			m_sprite = std::shared_ptr<HAPISPACE::Sprite>(HAPI_Sprites.MakeSprite(Textures::m_greenShipSnipe));
			break;
		}
		break;
	}
	m_sprite->SetFrameNumber(eShipSpriteFrame::eMaxHealth);
	m_sprite->GetTransformComp().SetOriginToCentreOfFrame();
}

OverWorld::OverWorld()
	: m_currentPlayer(0),
	m_selectNextPlayer(false),
	m_players(),
	m_GUI(),
	m_battle(),
	m_startBattle(false)
{
	GameEventMessenger::getInstance().subscribe(std::bind(&OverWorld::onReset, this), "OverWorld", GameEvent::eResetBattle);
	m_players.emplace_back(FactionName::eYellow, ePlayerType::eNone);
	
	m_GUI.reset(m_players[m_currentPlayer].m_entities);
}

OverWorld::~OverWorld()
{
	GameEventMessenger::getInstance().unsubscribe("OverWorld", GameEvent::eResetBattle);
}

void OverWorld::OnMouseEvent(EMouseEvent mouseEvent, const HAPI_TMouseData & mouseData)
{
	if (mouseEvent == EMouseEvent::eLeftButtonDown)
	{
		bool selectNextPlayer = false;
		bool resetPlayer = false;
		m_GUI.onLeftClick(mouseData, m_players[m_currentPlayer], selectNextPlayer, resetPlayer);

		if (OverWorldGUI::CURRENT_WINDOW == eLevelSelection)
		// put bool to stop this triggering more than once only wants to trigger on first enter
		{
			if (m_GUI.getLeftPlayerSelectionTrig())
			{
				m_GUI.setActivePlayers(m_players);
				onReset();
				m_GUI.setLeftPlayerSelectionTrig (false);
			}
		// create bool which triggers if leaving selection into map or leaving map into fleet
		}

		if(selectNextPlayer)
			++m_currentPlayer;
		while (OverWorldGUI::CURRENT_WINDOW == eShipSelection && m_currentPlayer < static_cast<int>(m_players.size()) && m_players[m_currentPlayer].m_type == ePlayerType::eAI)
		{
			//Call ai ship selection
			AI::handleShipSelection(m_players[m_currentPlayer].m_entities, m_players[m_currentPlayer].m_selectedEntities);
			++m_currentPlayer;

			if (m_currentPlayer < static_cast<int>(m_players.size()) && m_players[m_currentPlayer].m_type != ePlayerType::eAI)
			{
				m_GUI.setShipSelectionTrigger(true);
				m_GUI.reset(m_players[m_currentPlayer].m_entities);
			}
		}

		if (selectNextPlayer && m_currentPlayer < static_cast<int>(m_players.size()))
		{
			//++m_currentPlayer;

			if (m_currentPlayer < static_cast<int>(m_players.size()))
			{
				m_GUI.reset(m_players[m_currentPlayer].m_entities);
			}
		}

		if (resetPlayer)
		{
		//m_currentPlayer = 0;
		//m_GUI.setShipSelectionTrigger(false);
		//	m_GUI.reset(m_players[m_currentPlayer].m_entities);
			onReset();
			return;
		}
		if (m_currentPlayer >= static_cast<int>(m_players.size()))
		{
			m_startBattle = true;
			m_currentPlayer = 0;
			return;
		}
	}
	if (mouseEvent == EMouseEvent::eRightButtonDown)
	{
		//m_GUI.onRightClick(mouseData, m_players[m_currentPlayer]);
	}
}

void OverWorld::OnMouseMove(const HAPI_TMouseData & mouseData)
{
	m_GUI.onMouseMove(mouseData, m_players[m_currentPlayer]);
}

void OverWorld::render() const
{
	m_GUI.render(m_battle);
}

void OverWorld::update(float deltaTime)
{
	if (m_startBattle)
	{
		startBattle();
	}

	if (OverWorldGUI::CURRENT_WINDOW == eBattle)
	{
		m_battle.update(deltaTime);
	}
}

void OverWorld::startBattle()
{
	if (m_startBattle)
	{
		OverWorldGUI::CURRENT_WINDOW = eBattle;

		m_GUI.setShipSelectionTrigger(false);
		m_GUI.clear();
		m_battle.start(m_GUI.getSelectedMap(), m_players);
		m_startBattle = false;
	}
}

void OverWorld::onReset()
{
	m_GUI.setShipSelectionTrigger(false);
	m_currentPlayer = 0;
	m_selectNextPlayer = false;
	for (auto& player : m_players)
	{
		player.m_selectedEntities.clear();
	}
	//m_players.clear();
	//m_players.emplace_back(FactionName::eYellow);
	//m_players.emplace_back(FactionName::eBlue);
	//m_players.emplace_back(FactionName::eGreen);
	//m_players.emplace_back(FactionName::eRed);
	m_GUI.reset(m_players[m_currentPlayer].m_entities);
}