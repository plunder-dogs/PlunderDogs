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

enum class eGameState
{
	eMainMenu = 0,
	eMultiplayerLobby,
	eLevelSelection,
	eShipSelection,
	eBattle
};

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
	eGameState m_currentGameState;
	bool m_ready;
	Battle m_battle;
	sf::Event m_currentSFMLEvent;
	sf::Clock m_gameClock;
	float m_deltaTime;

	Sprite m_backgroundSprite;

	//Online Lobby

	void handleServerMessages();
	void handleInput();

	void assignFaction(eFactionName factionName, eFactionControllerType controllerType, const std::vector<eShipType>& shipsToAdd);
	eFactionName getLocalFactionName() const;
};