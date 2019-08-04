#include "Game.h"
#include "GameEventMessenger.h"

Game::Game(bool onlineGame)
	: m_factions(),
	m_window(sf::VideoMode(1920, 1080), "SFML_WINDOW", sf::Style::Default),
	m_onlineGame(onlineGame),
	m_gameLobbyActive(false),
	m_ready(false),
	m_battle(m_factions),
	m_currentSFMLEvent(),
	m_gameClock(),
	m_deltaTime(m_gameClock.restart().asSeconds())
{
	m_window.setFramerateLimit(120);

	if (onlineGame)
	{
		m_gameLobbyActive = true;
	}
	else
	{
		assignFaction(FactionName::eYellow, eControllerType::eLocalPlayer,
			{ eShipType::eFrigate, eShipType::eFrigate , eShipType::eFrigate , eShipType::eFrigate ,
			eShipType::eFrigate, eShipType::eFrigate });

		m_factions[static_cast<int>(FactionName::eRed)].m_factionName = FactionName::eRed;
		m_factions[static_cast<int>(FactionName::eRed)].m_controllerType = eControllerType::eAI;
		AIHandler::getInstance().loadShips(m_factions[static_cast<int>(FactionName::eRed)]);
		m_battle.startSinglePlayerGame("level1.tmx");
	}
}

void Game::run()
{
	while (m_battle.isRunning())
	{
		handleServerMessages();
		handleInput();
		handleGameLoop();

		m_deltaTime = m_gameClock.restart().asSeconds();
	}

	if (NetworkHandler::getInstance().isConnected() && m_onlineGame)
	{
		ServerMessage messageToSend(eMessageType::eDisconnect, getLocalFactionName());
		NetworkHandler::getInstance().sendServerMessage(messageToSend);
		NetworkHandler::getInstance().disconnect();
	}
}

void Game::handleServerMessages()
{
	if (!m_onlineGame)
	{
		return;
	}

	while (NetworkHandler::getInstance().hasMessages())
	{
		ServerMessage receivedServerMessage = NetworkHandler::getInstance().getServerMessage();
		if (receivedServerMessage.type == eMessageType::eEstablishConnection)
		{
			std::vector<eShipType> shipsToAdd;
			shipsToAdd.push_back(eShipType::eFrigate);
			shipsToAdd.push_back(eShipType::eFrigate);
			shipsToAdd.push_back(eShipType::eFrigate);
			shipsToAdd.push_back(eShipType::eFrigate);
			shipsToAdd.push_back(eShipType::eFrigate);
			shipsToAdd.push_back(eShipType::eFrigate);

			assignFaction(receivedServerMessage.faction, eControllerType::eLocalPlayer, shipsToAdd);
			for (auto& existingFaction : receivedServerMessage.existingFactions)
			{
				if (!existingFaction.existingShips.empty() && existingFaction.AIControlled)
				{
					assignFaction(existingFaction.factionName, eControllerType::eAI, existingFaction.existingShips);
				}
				else if (!existingFaction.existingShips.empty())
				{
					assignFaction(existingFaction.factionName, eControllerType::eRemotePlayer, existingFaction.existingShips);
				}
			}

			ServerMessage messageToSend(eMessageType::eNewPlayer, getLocalFactionName(), std::move(shipsToAdd));
			NetworkHandler::getInstance().sendServerMessage(messageToSend);
		}
		else if (receivedServerMessage.type == eMessageType::eNewPlayer)
		{
			if (receivedServerMessage.faction != getLocalFactionName())
			{
				assignFaction(receivedServerMessage.faction, eControllerType::eRemotePlayer, receivedServerMessage.shipsToAdd);
			}
		}
		else if (receivedServerMessage.type == eMessageType::eStartOnlineGame)
		{
			m_gameLobbyActive = false;
			m_battle.startOnlineGame(receivedServerMessage.levelName, receivedServerMessage.spawnPositions);
		}
		else if (receivedServerMessage.type == eMessageType::eRefuseConnection)
		{
			std::cout << "Connection Refused\n";
			m_window.close();
		}
		else if (!m_gameLobbyActive && (receivedServerMessage.type == eMessageType::eDeployShipAtPosition ||
			receivedServerMessage.type == eMessageType::eMoveShipToPosition ||
			receivedServerMessage.type == eMessageType::eAttackShipAtPosition ||
			receivedServerMessage.type == eMessageType::eClientDisconnected))
		{
			m_battle.receiveServerMessage(receivedServerMessage, getLocalFactionName());
		}
	}
}

void Game::handleInput()
{
	while (m_window.pollEvent(m_currentSFMLEvent))
	{
		if (m_currentSFMLEvent.type == sf::Event::Closed)
		{
			if (m_onlineGame)
			{
				ServerMessage messageToSend(eMessageType::eDisconnect, getLocalFactionName());
				NetworkHandler::getInstance().sendServerMessage(messageToSend);
				NetworkHandler::getInstance().disconnect();
				m_battle.quitGame();
			}
			m_window.close();
		}
		else if (m_currentSFMLEvent.type == sf::Event::KeyPressed)
		{
			if (m_currentSFMLEvent.key.code == sf::Keyboard::R && m_onlineGame && m_gameLobbyActive && !m_ready)
			{
				ServerMessage messageToSend(eMessageType::ePlayerReady, getLocalFactionName());
				NetworkHandler::getInstance().sendServerMessage(messageToSend);
				m_ready = true;
			}
		}
		if (m_battle.isRunning() && (!m_onlineGame || (m_onlineGame && !m_gameLobbyActive)))
		{
			m_battle.handleInput(m_window, m_currentSFMLEvent);
		}
	}
}

void Game::handleGameLoop()
{
	if (!m_battle.isRunning())
	{
		return;
	}

	if (m_onlineGame && !m_gameLobbyActive)
	{
		m_battle.update(m_deltaTime);

		m_window.clear();
		m_battle.render(m_window);
		m_window.display();
	}
	else if (!m_onlineGame)
	{
		m_battle.update(m_deltaTime);

		m_window.clear();
		m_battle.render(m_window);
		m_window.display();
	}
}

void Game::assignFaction(FactionName factionName, eControllerType controllerType, const std::vector<eShipType>& shipsToAdd)
{
	m_factions[static_cast<int>(factionName)].m_factionName = factionName;
	m_factions[static_cast<int>(factionName)].m_controllerType = controllerType;

	for (eShipType shipToAdd : shipsToAdd)
	{
		m_factions[static_cast<int>(factionName)].addShip(factionName, shipToAdd);
	}
}

FactionName Game::getLocalFactionName() const
{
	auto cIter = std::find_if(m_factions.cbegin(), m_factions.cend(), [](const auto& faction)
	{ return faction.m_controllerType == eControllerType::eLocalPlayer; });

	assert(cIter != m_factions.cend());
	return cIter->m_factionName;
}