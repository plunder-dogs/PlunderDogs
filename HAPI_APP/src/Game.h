#pragma once

#include "Textures.h"
#include "Battle.h"
#include "Utilities/XMLParser.h"
#include "UI/UILayer.h"
#include "NetworkHandler.h"
#include <assert.h>

class Game : private NonCopyable 
{
public:
	Game(const sf::Font& font);

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
	Battle m_battle;
	sf::Event m_currentSFMLEvent;
	sf::Clock m_gameClock;
	float m_deltaTime;
	
	//UI
	std::array<UILayer, static_cast<size_t>(eGameState::Total)> m_UILayers;
	sf::RectangleShape mouseShape;

	void handleServerMessages();
	void handleInput();
	void handleMainMenuInput(sf::IntRect mouseRect);
	void handleSinglePlayerFactionSelectionInput(sf::IntRect mouseRect);
	void render();
	void assignFaction(eFactionName factionName, eFactionControllerType controllerType, const std::vector<eShipType>& shipsToAdd);
	eFactionName getLocalControlledFaction() const;

	void quit();
};