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

void printFactions(const std::array<Faction, static_cast<size_t>(FactionName::eTotal)>& factions)
{
	for (auto& faction : factions)
	{
		if (faction.m_controllerType == eControllerType::None)
		{
			continue;
		}

		std::cout << static_cast<int>(faction.m_factionName) << "\n";
		std::cout << "Ships:\n";
		for (auto& ship : faction.getAllShips())
		{
			std::cout << static_cast<int>(ship.getShipType()) << "\n";
		}
	}
}

FactionName getLocalFactionName(const std::array<Faction, static_cast<size_t>(FactionName::eTotal)>& factions)
{
	auto cIter = std::find_if(factions.cbegin(), factions.cend(), [](const auto& faction) 
		{ return faction.m_controllerType == eControllerType::eLocalPlayer; });
	
	assert(cIter != factions.cend());
	return cIter->m_factionName;
}

void assignFaction(std::array<Faction, static_cast<size_t>(FactionName::eTotal)>& factions, FactionName factionName, eControllerType controllerType,
	std::vector<eShipType>& shipsToAdd)
{
	factions[static_cast<int>(factionName)].m_factionName = factionName;
	factions[static_cast<int>(factionName)].m_controllerType = controllerType;
	
	for (eShipType shipToAdd : shipsToAdd)
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
	NetworkHandler::getInstance().connect();
	bool gameLobbyActive = true;
	bool localPlayerReady = false;
	sf::Clock gameClock;
	sf::Event currentEvent;
	float deltaTime = gameClock.restart().asSeconds();
	while (window.isOpen())
	{
		//Handle Server Messages
		NetworkHandler::getInstance().listenToServer();
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
				for (auto& existingFaction : serverMessage.existingFactions)
				{
					assignFaction(factions, existingFaction.factionName, eControllerType::eRemotePlayer, existingFaction.existingShips);
				}

				ServerMessage messageToSend(eMessageType::eNewPlayer, getLocalFactionName(factions), std::move(shipsToAdd));
				NetworkHandler::getInstance().sendServerMessage(messageToSend);
			}
			else if (serverMessage.type == eMessageType::eNewPlayer)
			{
				if (serverMessage.faction != getLocalFactionName(factions))
				{
					assignFaction(factions, serverMessage.faction, eControllerType::eRemotePlayer, serverMessage.shipsToAdd);

					printFactions(factions);
				}
			}
			else if (serverMessage.type == eMessageType::eStartOnlineGame)
			{
				gameLobbyActive = false;
				battle.startOnlineGame("Level1.tmx", serverMessage.spawnPositions);
			}
			else if (serverMessage.type == eMessageType::eRefuseConnection)
			{
				//Exit Game
				std::cout << "Connection Refused\n";
				window.close();
				return 0;
			}
			else if (!gameLobbyActive && serverMessage.type == eMessageType::eDeployShipAtPosition,
				serverMessage.type == eMessageType::eMoveShipToPosition,
				serverMessage.type == eMessageType::eAttackShipAtPosition)
			{
				battle.receiveServerMessage(serverMessage);
			}
		}

		//Handle Input
		while (window.pollEvent(currentEvent))
		{
			if (currentEvent.type == sf::Event::Closed)
			{
				window.close();
			}
			else if (currentEvent.type == sf::Event::KeyPressed)
			{
				if (currentEvent.key.code == sf::Keyboard::R && !localPlayerReady)
				{
					ServerMessage messageToSend(eMessageType::ePlayerReady, getLocalFactionName(factions));
					NetworkHandler::getInstance().sendServerMessage(messageToSend);
					localPlayerReady = true;
				}
			}
			if (!gameLobbyActive)
			{
				battle.handleInput(window, currentEvent);
			}
		}
	
		if (!gameLobbyActive)
		{
			battle.update(deltaTime);

			window.clear();
			battle.render(window);
			window.display();
		}

		deltaTime = gameClock.restart().asSeconds();
	}

	ServerMessage messageToSend(eMessageType::eDisconnect, getLocalFactionName(factions));
	NetworkHandler::getInstance().sendServerMessage(messageToSend);
	NetworkHandler::getInstance().disconnect();

	return 0;
}