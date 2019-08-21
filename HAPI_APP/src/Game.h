#pragma once

#include "Textures.h"
#include "Battle.h"
#include "Utilities/XMLParser.h"
#include "UI/UILayer.h"
#include "NetworkHandler.h"
#include <assert.h>

enum class eGameState
{
	eMainMenu = 0,
	eSinglePlayerFactionSelect,
	eMultiplayerLobby,
	eLevelSelection,
	eShipSelection,
	eBattle,
	Total = eBattle + 1
};

class Game : private NonCopyable 
{
public:
	Game(const sf::Font& font);
	~Game();

	void run();

private:
	std::array<Faction, static_cast<size_t>(eFactionName::eTotal)> m_factions
	{
		eFactionName::eYellow, 
		eFactionName::eBlue, 
		eFactionName::eGreen, 
		eFactionName::eRed
	};
	sf::RenderWindow m_window;
	eGameState m_currentGameState;
	bool m_ready;
	std::unique_ptr<Battle> m_battle;
	sf::Event m_currentSFMLEvent;
	sf::Clock m_gameClock;
	float m_deltaTime;
	std::array<UILayer, static_cast<size_t>(eGameState::Total)> m_UILayers;
	sf::RectangleShape m_mouseShape;

	void handleServerMessages();
	void render();
	void assignFaction(eFactionName factionName, eFactionControllerType controllerType, const std::vector<eShipType>& shipsToAdd);
	void fillFaction(eFactionName factionName, int frameID);
	eFactionName getLocalControlledFaction() const;
	void quit();
	void resetAllFactions();

	//UI
	void handleInput();
	void handleMainMenuInput(sf::IntRect mouseRect);
	void handleSinglePlayerFactionSelectionInput(sf::IntRect mouseRect);
	void handleLevelSelectionInput(sf::IntRect mouseRect);
	void handleBattleInput(sf::IntRect mouseRect);
	void handleMultiplayerLobbyInput(sf::IntRect mouseRect);
	//UI Events
	void onAllFactionsFinishedDeployment(GameEvent gameEvent);
	void onNewFactionTurn(GameEvent gameEvent);
	void onHideEndPhaseButton(GameEvent gameEvent);
	void onShowEndPhaseButton(GameEvent gameEvent);
};