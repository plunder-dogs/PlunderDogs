#include  "Overworld.h"
#include "Textures.h"
#include "GameEventMessenger.h"
#include "AI.h"

//ENTITY
ShipProperties::ShipProperties(FactionName factionName, ShipType entityType) 
	: m_sprite(),
	m_selectedSprite(HAPI_Sprites.MakeSprite(Textures::m_thing)),
	m_type(entityType),
	m_movementPoints(0),
	m_health(0),
	m_range(0),
	m_damage(0),
	m_weaponType()
{
	
}

OverWorld::OverWorld()
	: m_currentPlayer(0),
	m_selectNextPlayer(false),
	m_factions(),
	m_GUI(),
	m_battle(),
	m_startBattle(false)
{
	GameEventMessenger::getInstance().subscribe(std::bind(&OverWorld::onReset, this), "OverWorld", GameEvent::eResetBattle);
	m_factions.emplace_back(FactionName::eYellow, ePlayerType::eNone);
	
	m_GUI.reset(m_factions[m_currentPlayer].m_ships);
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
		m_GUI.onLeftClick(mouseData, m_factions[m_currentPlayer], selectNextPlayer, resetPlayer);

		if (OverWorldGUI::CURRENT_WINDOW == eLevelSelection)
		// put bool to stop this triggering more than once only wants to trigger on first enter
		{
			if (m_GUI.getLeftPlayerSelectionTrig())
			{
				m_GUI.setActivePlayers(m_factions);
				onReset();
				m_GUI.setLeftPlayerSelectionTrig (false);
			}
		// create bool which triggers if leaving selection into map or leaving map into fleet
		}

		if(selectNextPlayer)
			++m_currentPlayer;
		while (OverWorldGUI::CURRENT_WINDOW == eShipSelection && m_currentPlayer < static_cast<int>(m_factions.size()) && m_factions[m_currentPlayer].m_type == ePlayerType::eAI)
		{
			//Call ai ship selection
			//AI::handleShipSelection(m_players[m_currentPlayer].m_entities, m_players[m_currentPlayer].m_selectedEntities);
			++m_currentPlayer;

			if (m_currentPlayer < static_cast<int>(m_factions.size()) && m_factions[m_currentPlayer].m_type != ePlayerType::eAI)
			{
				m_GUI.setShipSelectionTrigger(true);
				m_GUI.reset(m_factions[m_currentPlayer].m_ships);
			}
		}

		if (selectNextPlayer && m_currentPlayer < static_cast<int>(m_factions.size()))
		{
			//++m_currentPlayer;

			if (m_currentPlayer < static_cast<int>(m_factions.size()))
			{
				m_GUI.reset(m_factions[m_currentPlayer].m_ships);
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
		if (m_currentPlayer >= static_cast<int>(m_factions.size()))
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
	m_GUI.onMouseMove(mouseData, m_factions[m_currentPlayer]);
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
		m_battle.start(m_GUI.getSelectedMap(), m_factions);
		m_startBattle = false;
	}
}

void OverWorld::onReset()
{
	m_GUI.setShipSelectionTrigger(false);
	m_currentPlayer = 0;
	m_selectNextPlayer = false;
	for (auto& player : m_factions)
	{
		player.m_ships.clear();
	}
	//m_players.clear();
	//m_players.emplace_back(FactionName::eYellow);
	//m_players.emplace_back(FactionName::eBlue);
	//m_players.emplace_back(FactionName::eGreen);
	//m_players.emplace_back(FactionName::eRed);
	m_GUI.reset(m_factions[m_currentPlayer].m_ships);
}

PlayerDetails::PlayerDetails()
{

}

void PlayerDetails::addShip(ShipType shipType)
{
	m_ships.emplace_back(m_factionName, shipType);
}