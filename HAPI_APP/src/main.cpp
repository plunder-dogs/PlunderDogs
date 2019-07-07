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
#endif // C++_NOTES

FactionName getLocalFactionName(std::array<Faction, static_cast<size_t>(FactionName::eTotal)>& factions)
{
	for (auto& faction : factions)
	{
		if (faction.m_controllerType == eControllerType::eLocalPlayer)
		{
			return faction.m_factionName;
		}
	}
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

//void assignRemoteFaction(std::array<Faction, static_cast<size_t>(FactionName::eTotal)>& factions, FactionName remoteFactionName,
//	std::vector<eShipType>& remoteFactionShips)
//{
//	factions[static_cast<int>(remoteFactionName)].m_factionName = remoteFactionName;
//	factions[static_cast<int>(remoteFactionName)].m_controllerType = eControllerType::eRemotePlayer;
//
//	for (eShipType shipToAdd : remoteFactionShips)
//	{
//		factions[static_cast<int>(remoteFactionName)].addShip(remoteFactionName, shipToAdd);
//	}
//}
//
//void assignLocalFaction(std::array<Faction, static_cast<size_t>(FactionName::eTotal)>& factions, FactionName factionName,
//	std::vector<eShipType>& remoteFactionShips)
//{
//	factions[static_cast<int>(factionName)].m_factionName = factionName;
//	factions[static_cast<int>(factionName)].m_controllerType = eControllerType::eLocalPlayer;
//	
//	for (eShipType shipToAdd : remoteFactionShips)
//	{
//		factions[static_cast<int>(factionName)].addShip(factionName, shipToAdd);
//	}
//}

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
	//Battle battle(factions);
	bool gameLobby = true;
	int shipLoadout = 0;

	std::cout << "Selected Ship Loadout: " << "\n";
	std::cin >> shipLoadout;
	assert(shipLoadout == 1 || shipLoadout == 2);
	NetworkHandler networkHandler;
	networkHandler.connect();

	sf::Vector2u windowSize(1920, 1080);
	sf::RenderWindow window(sf::VideoMode(windowSize.x, windowSize.y), "SFML_WINDOW", sf::Style::Default);
	window.setFramerateLimit(120);

	sf::Clock gameClock;
	sf::Event currentEvent;
	float deltaTime = gameClock.restart().asSeconds();
	while (window.isOpen())
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
							//NetworkHandler::getInstance().sendServerMessage({ eMessageType::ePlayerReady, faction.m_factionName });
						}
					}
				}
			}

			if (!gameLobby)
			{
				//battle.handleInput(window, currentEvent);
			}
		}
	
		if (gameLobby)
		{
			while (networkHandler.isMessages())
			{
				ServerMessage serverMessage = networkHandler.getServerMessage();
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
					//sf::Packet packetToSend;
					//FactionName localFactionName = getLocalFactionName(factions);
					//packetToSend << static_cast<int>(eMessageType::eNewPlayer) << static_cast<int>(localFactionName) <<
					//	static_cast<int>(shipsToAdd.size()) << shipsToAdd;

					//NetworkHandler::getInstance().sendServerMessage(packetToSend);
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
					//battle.start("Level1.tmx", gameLobby);
				}
			}
		}
		else
		{
			//battle.update(deltaTime);

			window.clear();
			//battle.render(window);
			window.display();

		}

		deltaTime = gameClock.restart().asSeconds();
	}

	networkHandler.disconnect(getLocalFactionName(factions));

	int i = 0;

	return 0;
}