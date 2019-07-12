#include "Textures.h"
#include "Battle.h"
#include "AI.h"
#include <array>
#include "SFML/Graphics.hpp"
#include "Utilities/XMLParser.h"
#include <iostream>
#include "NetworkHandler.h"
#include <assert.h>

#ifdef C++_NOTES
//Copy Ellision
//Return Value Optimzation
//Stack Frames
//https://www.youtube.com/channel/UCSX3MR0gnKDxyXAyljWzm0Q/playlists
//https://www.reddit.com/r/cpp/comments/byi5m3/books_on_memory_management_techniques/
//https://stackoverflow.com/questions/37960797/threads-termination-of-infinite-loop-thread-in-c
//branch misprediction 
//pipeline stall - https://gameprogrammingpatterns.com/data-locality.html
//Pointer Chasing
//L1,L2,L3, L4 cache
//Pointer Prediction
//Branch Prediction
//Memory prefeching
//Threading debug confusing for timings
//debug by printing, if statements
#endif // C++_NOTES

FactionName getLocalFactionName(std::array<Faction, static_cast<size_t>(FactionName::eTotal)>& factions)
{
	auto cIter = std::find_if(factions.cbegin(), factions.cend(), [](const auto& faction) 
		{ return faction.m_controllerType == eControllerType::eLocalPlayer; });
	
	assert(cIter != factions.cend());
	return cIter->m_factionName;
}

void assignFaction(std::array<Faction, static_cast<size_t>(FactionName::eTotal)>& factions, FactionName factionName, eControllerType controllerType,
	std::vector<eShipType>& remoteFactionShips)
{
	factions[static_cast<int>(factionName)].m_factionName = factionName;
	factions[static_cast<int>(factionName)].m_controllerType = controllerType;

	for (eShipType shipToAdd : remoteFactionShips)
	{
		factions[static_cast<int>(factionName)].addShip(factionName, shipToAdd);
	}
}

int main()
{
	std::cout << "Selected Ship Loadout: " << "\n";
	int shipLoadout = 0;
	std::cin >> shipLoadout;
	assert(shipLoadout == 1 || shipLoadout == 2);

	sf::Vector2u windowSize(1920, 1080);
	sf::RenderWindow window(sf::VideoMode(windowSize.x, windowSize.y), "SFML_WINDOW", sf::Style::Default);
	window.setFramerateLimit(120);

	Textures::getInstance().loadAllTextures();
	std::array<Faction, static_cast<size_t>(FactionName::eTotal)> factions;
	Battle battle(factions);
	bool gameLobby = true;

	sf::Clock gameClock;
	sf::Event currentEvent;
	float deltaTime = gameClock.restart().asSeconds();
	while (window.isOpen())
	{
		NetworkHandler::getInstance().listenToServer();
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
			while (NetworkHandler::getInstance().hasMessages())
			{
				ServerMessage serverMessage = NetworkHandler::getInstance().getServerMessage();
				if (serverMessage.type == eMessageType::eEstablishConnection)
				{
					std::vector<eShipType> shipsToAdd;
					if (shipLoadout == 1)
					{
						shipsToAdd.push_back(eShipType::eFrigate);
						shipsToAdd.push_back(eShipType::eFrigate);
						shipsToAdd.push_back(eShipType::eFrigate);
					}
					else if (shipLoadout == 2)
					{
						shipsToAdd.push_back(eShipType::eTurtle);
						shipsToAdd.push_back(eShipType::eTurtle);
						shipsToAdd.push_back(eShipType::eTurtle);
					}

					assignFaction(factions, serverMessage.faction, eControllerType::eLocalPlayer, shipsToAdd);

					sf::Packet packetToSend;
					FactionName localFactionName = getLocalFactionName(factions);
					packetToSend << static_cast<int>(eMessageType::eNewPlayer) << static_cast<int>(localFactionName) <<
						static_cast<int>(shipsToAdd.size()) << shipsToAdd;

					NetworkHandler::getInstance().sendServerMessage(packetToSend);
				}
				else if (serverMessage.type == eMessageType::eNewPlayer)
				{
					if (serverMessage.faction != getLocalFactionName(factions))
					{
						assignFaction(factions, serverMessage.faction, eControllerType::eRemotePlayer, serverMessage.shipsToAdd);
					}
				}
				else if (serverMessage.type == eMessageType::eStartGame)
				{
					gameLobby = false;
					battle.start("Level1.tmx", gameLobby);
				}
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

	sf::Packet packetToSend;
	packetToSend << static_cast<int>(eMessageType::eDisconnect) << static_cast<int>(getLocalFactionName(factions));
	NetworkHandler::getInstance().sendServerMessage(packetToSend);
	NetworkHandler::getInstance().disconnect();

	return 0;
}