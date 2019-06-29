#include "Textures.h"
#include "Battle.h"
#include "AI.h"
#include <array>
#include "SFML/Graphics.hpp"
#include "Utilities/XMLParser.h"
#include <iostream>
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

void assignRemoteFaction(std::array<Faction, static_cast<size_t>(FactionName::eTotal)>& factions, FactionName remoteFactionName,
	std::vector<eShipType>& remoteFactionShips)
{
	factions[static_cast<int>(remoteFactionName)].m_factionName = remoteFactionName;
	factions[static_cast<int>(remoteFactionName)].m_controllerType = eControllerType::eRemotePlayer;

	for (eShipType shipToAdd : remoteFactionShips)
	{
		factions[static_cast<int>(remoteFactionName)].addShip(remoteFactionName, shipToAdd);
	}
}

void assignLocalFaction(std::array<Faction, static_cast<size_t>(FactionName::eTotal)>& factions, FactionName factionName,
	std::vector<eShipType>& remoteFactionShips)
{
	factions[static_cast<int>(factionName)].m_factionName = factionName;
	factions[static_cast<int>(factionName)].m_controllerType = eControllerType::eLocalPlayer;
	
	for (eShipType shipToAdd : remoteFactionShips)
	{
		factions[static_cast<int>(factionName)].addShip(factionName, shipToAdd);
	}
}

void startSinglePlayer(std::array<Faction, static_cast<size_t>(FactionName::eTotal)>& factions)
{
	factions[static_cast<int>(FactionName::eYellow)].m_factionName = FactionName::eYellow;
	factions[static_cast<int>(FactionName::eYellow)].m_controllerType = eControllerType::eLocalPlayer;
	factions[static_cast<int>(FactionName::eYellow)].addShip(FactionName::eYellow, eShipType::eFrigate);
	factions[static_cast<int>(FactionName::eYellow)].addShip(FactionName::eYellow, eShipType::eFrigate);

	factions[static_cast<int>(FactionName::eRed)].m_factionName = FactionName::eRed;
	factions[static_cast<int>(FactionName::eRed)].m_controllerType = eControllerType::eAI;

	AI::loadShips(factions[static_cast<int>(FactionName::eRed)]);
}

int main()
{
	Textures::getInstance().loadAllTextures();
	std::array<Faction, static_cast<size_t>(FactionName::eTotal)> factions;
	Battle battle(factions);
	std::cout << "1. Single Player\n";
	std::cout << "2. Multiplayer\n";
	int userInput = 0;
	std::cin >> userInput;
	bool gameLobby = true;
	if (userInput == 1)
	{
		startSinglePlayer(factions);
		gameLobby = false;
		battle.start("Level1.tmx", gameLobby);
	}
	else if (userInput == 2)
	{
		NetworkHandler::getInstance().connect();
	}

	sf::Vector2u windowSize(1920, 1080);
	sf::RenderWindow window(sf::VideoMode(windowSize.x, windowSize.y), "SFML_WINDOW", sf::Style::Default);
	window.setFramerateLimit(120);

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
			else if (currentEvent.type == sf::Event::KeyPressed)
			{
				if (currentEvent.key.code == sf::Keyboard::R)
				{
					for (auto& faction : factions)
					{
						if (faction.m_controllerType == eControllerType::eLocalPlayer)
						{
							NetworkHandler::getInstance().sendServerMessage({ eMessageType::ePlayerReady, faction.m_factionName });
						}
					}
				}
			}

			if (!gameLobby)
			{
				battle.handleInput(window, currentEvent);
			}
		}
	
		if (gameLobby)
		{
			if (!NetworkHandler::getInstance().getServerMessages().empty())
			{
				for (auto message : NetworkHandler::getInstance().getServerMessages())
				{
					if (message.type == eMessageType::eEstablishConnection)
					{
						assignLocalFaction(factions, message.factionSentFrom, message.shipsToAdd);
					}
					else if (message.type == eMessageType::eNewRemoteConnection)
					{
						assignRemoteFaction(factions, message.factionSentFrom, message.shipsToAdd);
					}
					else if (message.type == eMessageType::eStartGame)
					{
						gameLobby = false;
						battle.start("Level1.tmx", gameLobby);
					}
				}

				NetworkHandler::getInstance().getServerMessages().clear();
			}
		}
		else
		{
			battle.update(deltaTime);

			window.clear();
			battle.render(window);
			window.display();

		}

		deltaTime = gameClock.restart().asSeconds();
	}

	NetworkHandler::getInstance().disconnect();

	return 0;
}