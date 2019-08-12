#pragma once

#include "Textures.h"
#include "Battle.h"
#include "AIHandler.h"
#include <array>
#include "SFML/Graphics.hpp"
#include "Utilities/XMLParser.h"
#include <iostream>
#include "NetworkHandler.h"
#include <assert.h>

constexpr size_t MAX_SHIP_SPRITES = 24;

class Game : private NonCopyable 
{
public:
	Game();

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
	bool m_onlineGame;
	bool m_gameLobbyActive;
	bool m_ready;
	Battle m_battle;
	sf::Event m_currentSFMLEvent;
	sf::Clock m_gameClock;
	float m_deltaTime;
	Sprite m_backgroundSprite;

	//Online Lobby

	void fillLobby();

	void handleServerMessages();
	void handleInput();
	void handleGameLoop();
	void renderLobby();

	void assignFaction(eFactionName factionName, eFactionControllerType controllerType, const std::vector<eShipType>& shipsToAdd);
	eFactionName getLocalFactionName() const;
};