#include "Game.h"
#include "GameEventMessenger.h"
#include "Sprite.h"
#include "Textures.h"
#include "Texture.h"

Game::Game()
	: m_window(sf::VideoMode(1920, 1080), "SFML_WINDOW", sf::Style::Default),
	m_onlineGame(false)
	m_gameLobbyActive(false),
	m_ready(false),
	m_battle(m_factions),
	m_currentSFMLEvent(),
	m_gameClock(),
	m_deltaTime(m_gameClock.restart().asSeconds()),
	m_backgroundSprite(Textures::getInstance().getTexture(BACKGROUND))
{
	m_window.setFramerateLimit(120);

	//if (onlineGame)
	//{
	//	m_gameLobbyActive = true;
	//}
	//else
	//{
	//	assignFaction(eFactionName::eYellow, eFactionControllerType::eLocalPlayer,
	//		{ eShipType::eFrigate, eShipType::eFrigate , eShipType::eFrigate , eShipType::eFrigate ,
	//		eShipType::eFrigate, eShipType::eFrigate });

	//	m_factions[static_cast<int>(eFactionName::eRed)].m_controllerType = eFactionControllerType::eAI;
	//	AIHandler::getInstance().loadShips(m_factions[static_cast<int>(eFactionName::eRed)]);
	//	m_battle.startSinglePlayerGame("level3.tmx");
	//}
}

void Game::run()
{
	while (m_window.isOpen())
	{
		if(m_battle.isRunning())
		{
			handleServerMessages();
			handleInput();
			handleGameLoop();

			m_deltaTime = m_gameClock.restart().asSeconds();
		}
	}




	if (NetworkHandler::getInstance().isConnected() && m_onlineGame)
	{
		ServerMessage messageToSend(eMessageType::eDisconnect, getLocalFactionName());
		NetworkHandler::getInstance().sendServerMessage(messageToSend);
		NetworkHandler::getInstance().disconnect();
	}
}

void Game::fillLobby()
{
	assert(m_onlineGame);
}

void Game::handleServerMessages()
{
	if (!m_onlineGame)
	{
		return;
	}
	

	NetworkHandler::getInstance().handleBackLog();

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

			assignFaction(receivedServerMessage.faction, eFactionControllerType::eLocalPlayer, shipsToAdd);
			for (auto& existingFaction : receivedServerMessage.existingFactions)
			{
				if (!existingFaction.existingShips.empty() && existingFaction.AIControlled)
				{
					assignFaction(existingFaction.factionName, eFactionControllerType::eAI, existingFaction.existingShips);
				}
				else if (!existingFaction.existingShips.empty())
				{
					assignFaction(existingFaction.factionName, eFactionControllerType::eRemotePlayer, existingFaction.existingShips);
				}
			}

			ServerMessage messageToSend(eMessageType::eNewPlayer, getLocalFactionName(), std::move(shipsToAdd));
			NetworkHandler::getInstance().sendServerMessage(messageToSend);
		}
		else if (receivedServerMessage.type == eMessageType::eNewPlayer)
		{
			if (receivedServerMessage.faction != getLocalFactionName())
			{
				assignFaction(receivedServerMessage.faction, eFactionControllerType::eRemotePlayer, receivedServerMessage.shipsToAdd);
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
			if (getLocalFactionName() != receivedServerMessage.faction)
			{
				m_battle.receiveServerMessage(receivedServerMessage);
			}
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

	if (m_battle.isRunning())
	{
		//m_backgroundSprite.render(m_window);
		if (m_onlineGame && !m_gameLobbyActive)
		{
			m_battle.update(m_deltaTime);

			m_window.clear();
		}
		else if (!m_onlineGame)
		{
			m_battle.update(m_deltaTime);
		}
	}
}

void Game::renderLobby()
{
	if (!m_onlineGame)
	{
		return;
	}

	sf::Vector2f factionTextPosition(100, 100);
	sf::Text factionNameText;
	std::string factionText;
	for (const auto& faction : m_factions)
	{
		switch (faction.m_factionName)
		{
		case eFactionName::eYellow :
			break;

		case eFactionName::eBlue :
			break;

		case eFactionName::eGreen :
			break;

		case eFactionName::eRed :
			break;
		}
	}
}

void Game::assignFaction(eFactionName factionName, eFactionControllerType controllerType, const std::vector<eShipType>& shipsToAdd)
{
	m_factions[static_cast<int>(factionName)].m_controllerType = controllerType;

	for (eShipType shipToAdd : shipsToAdd)
	{
		m_factions[static_cast<int>(factionName)].addShip(factionName, shipToAdd);
	}
}

eFactionName Game::getLocalFactionName() const
{
	auto cIter = std::find_if(m_factions.cbegin(), m_factions.cend(), [](const auto& faction)
	{ return faction.m_controllerType == eFactionControllerType::eLocalPlayer; });

	assert(cIter != m_factions.cend());
	return cIter->m_factionName;
}