#pragma once

#include "Textures.h"
#include "Battle.h"
#include "Utilities/XMLParser.h"
#include "UI/UILayer.h"
#include "NetworkHandler.h"
#include <assert.h>

constexpr size_t MAX_UI_LAYERS = 4;

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
	std::array<UILayer, MAX_UI_LAYERS> m_UILayers
	{
		//UI Layer owned by GameState
		eGameState::eMainMenu,
		eGameState::eSinglePlayerFactionSelect,
		eGameState::eMultiplayerLobby,
		eGameState::eLevelSelection
	};
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
	void handleMainMenuInput(sf::Vector2i mousePosition);
	void handleSinglePlayerFactionSelectionInput(sf::Vector2i mousePosition);
	void handleLevelSelectionInput(sf::Vector2i mousePosition);
	void handleMultiplayerLobbyInput(sf::Vector2i mousePosition);

	void onQuitGame(GameEvent gameEvent);
};