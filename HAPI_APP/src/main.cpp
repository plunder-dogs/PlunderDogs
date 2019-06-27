#include "Textures.h"
#include "Battle.h"
#include "AI.h"
#include <array>
#include "SFML/Graphics.hpp"
#include "Utilities/XMLParser.h"
#include <iostream>
#include <SFML/Network.hpp>
#include "NetworkHandler.h"

#ifdef C++_NOTES
//Copy Ellision
//Return Value Optimzation
//Stack Frames
//https://www.youtube.com/channel/UCSX3MR0gnKDxyXAyljWzm0Q/playlists
//https://www.reddit.com/r/cpp/comments/byi5m3/books_on_memory_management_techniques/
//branch misprediction 
//pipeline stall - https://gameprogrammingpatterns.com/data-locality.html
//Pointer Chasing
//L1,L2,L3, L4 cache
//Pointer Prediction
//Branch Prediction
//Memory prefeching
#endif // C++_NOTES

void assignFaction(std::array<Faction, static_cast<size_t>(FactionName::eTotal)>& factions, FactionName factionName, ePlayerType playerType)
{
	factions[static_cast<int>(factionName)].m_factionName = factionName;
	factions[static_cast<int>(factionName)].m_playerType = playerType;
}

void startSinglePlayer(std::array<Faction, static_cast<size_t>(FactionName::eTotal)>& factions)
{
	factions[static_cast<int>(FactionName::eYellow)].m_factionName = FactionName::eYellow;
	factions[static_cast<int>(FactionName::eYellow)].m_playerType = ePlayerType::eLocalPlayer;
	factions[static_cast<int>(FactionName::eYellow)].addShip(FactionName::eYellow, eShipType::eFrigate);
	factions[static_cast<int>(FactionName::eYellow)].addShip(FactionName::eYellow, eShipType::eFrigate);

	factions[static_cast<int>(FactionName::eRed)].m_factionName = FactionName::eRed;
	factions[static_cast<int>(FactionName::eRed)].m_playerType = ePlayerType::eAI;

	AI::loadShips(factions[static_cast<int>(FactionName::eRed)]);
}

int main()
{
	std::array<Faction, static_cast<size_t>(FactionName::eTotal)> factions;
	std::cout << "1. Single Player\n";
	std::cout << "2. Multiplayer\n";
	int userInput = 0;
	std::cin >> userInput;
	if (userInput == 1)
	{
		startSinglePlayer(factions);
	}
	else if (userInput == 2)
	{
		NetworkHandler::getInstance().connect();
	}

	bool gameLobby = true;
	while (gameLobby)
	{
		if (!NetworkHandler::getInstance().getServerMessages().empty())
		{
			for (auto message : NetworkHandler::getInstance().getServerMessages())
			{
				if (message.type == eMessageType::eEstablishConnection)
				{
					assignFaction(factions, message.factionName, ePlayerType::eLocalPlayer);
				}
			}

			NetworkHandler::getInstance().getServerMessages().clear();
		}
	}

	sf::Vector2u windowSize(1920, 1080);
	sf::RenderWindow window(sf::VideoMode(windowSize.x, windowSize.y), "SFML_WINDOW", sf::Style::Default);
	window.setFramerateLimit(120);

	Textures::getInstance().loadAllTextures();

	Battle battle(factions);
	battle.start("Level1.tmx");

	sf::Clock gameClock;
	sf::Event currentEvent;
	float deltaTime = gameClock.restart().asSeconds();
	while (battle.isRunning())
	{
		while (window.pollEvent(currentEvent))
		{
			if (currentEvent.type == sf::Event::Closed)
			{
				window.close();
			}

			battle.handleInput(window, currentEvent);
		}
		
		battle.update(deltaTime);

		window.clear();
		battle.render(window);
		window.display();

		deltaTime = gameClock.restart().asSeconds();
	}

	return 0;
}