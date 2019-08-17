#include "Game.h"
#include "GameEventMessenger.h"
#include "Sprite.h"
#include "Textures.h"
#include "Texture.h"

Game::Game(const sf::Font& font)
	: m_window(sf::VideoMode(1920, 1080), "SFML_WINDOW", sf::Style::Default),
	m_currentGameState(eGameState::eMainMenu),
	m_ready(false),
	m_battle(m_factions),
	m_currentSFMLEvent(),
	m_gameClock(),
	m_deltaTime(m_gameClock.restart().asSeconds()),
	m_backgroundSprite(Textures::getInstance().getTexture(BACKGROUND), true, false),
	m_font(font)
{
	//main menu
	m_mainMenuTextBoxes.reserve(3);
	m_mainMenuTextBoxes.emplace_back("Play Singleplayer", m_font, sf::Vector2f(600, 600), eUITextBoxName::ePlaySinglePlayer);
	m_mainMenuTextBoxes.emplace_back("Play Multiplayer", m_font, sf::Vector2f(600, 680), eUITextBoxName::ePlayerMultiplayer);
	m_mainMenuTextBoxes.emplace_back("Quit", m_font, sf::Vector2f(600, 760), eUITextBoxName::eQuit);

	//Single player Faction Select
	m_singlePlayerButtons.reserve(4);
	m_singlePlayerButtons.emplace_back(Textures::getInstance().getTexture("blueSelectBtn.xml"), sf::Vector2i(400, 250), eUIButtonName::eBlueFactionSelect);
	m_singlePlayerButtons.emplace_back(Textures::getInstance().getTexture("greenSelectBtn.xml"), sf::Vector2i(600, 250), eUIButtonName::eGreenFactionSelect);
	m_singlePlayerButtons.emplace_back(Textures::getInstance().getTexture("redSelectBtn.xml"), sf::Vector2i(800, 250), eUIButtonName::eRedFactionSelect);
	m_singlePlayerButtons.emplace_back(Textures::getInstance().getTexture("yellowSelectBtn.xml"), sf::Vector2i(1000, 250), eUIButtonName::eYellowFactionSelect);

	m_window.setFramerateLimit(120);

	mouseShape.setFillColor(sf::Color::Red);
	mouseShape.setSize(sf::Vector2f(25, 25));

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
		handleServerMessages();
		handleInput();

		render();
		//if (m_onlineGame && !m_gameLobbyActive)
		//{
		//	m_battle.update(m_deltaTime);
		//	m_window.clear();
		//}
		//else if (!m_onlineGame)
		//{
		//	m_battle.update(m_deltaTime);
		//}

		m_deltaTime = m_gameClock.restart().asSeconds();
	}
	
	if (NetworkHandler::getInstance().isConnectedToServer())
	{
		NetworkHandler::getInstance().sendMessageToServer(ServerMessage(eMessageType::eDisconnect, getLocalControlledFaction()));
		NetworkHandler::getInstance().disconnectFromServer();
	}
}

void Game::handleServerMessages()
{
	if (!NetworkHandler::getInstance().isConnectedToServer())
	{
		return;
	}
		
	NetworkHandler::getInstance().handleBackLog();
	while (NetworkHandler::getInstance().hasMessages())
	{
		ServerMessage receivedServerMessage = NetworkHandler::getInstance().getServerMessage();
		switch (receivedServerMessage.type)
		{
		case eMessageType::eEstablishConnection :
		{
			std::vector<eShipType> shipsToAdd;
			shipsToAdd.push_back(eShipType::eFrigate);
			shipsToAdd.push_back(eShipType::eFrigate);
			shipsToAdd.push_back(eShipType::eFrigate);
			shipsToAdd.push_back(eShipType::eFrigate);
			shipsToAdd.push_back(eShipType::eFrigate);
			shipsToAdd.push_back(eShipType::eFrigate);

			assignFaction(receivedServerMessage.faction, eFactionControllerType::eLocalPlayer, shipsToAdd);
			for (const auto& existingFaction : receivedServerMessage.existingFactions)
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

			ServerMessage messageToSend(eMessageType::eNewPlayer, getLocalControlledFaction(), std::move(shipsToAdd));
			NetworkHandler::getInstance().sendMessageToServer(messageToSend);
			break;
		}
		case eMessageType::eNewPlayer :
			if (receivedServerMessage.faction != getLocalControlledFaction())
			{
				assignFaction(receivedServerMessage.faction, eFactionControllerType::eRemotePlayer, receivedServerMessage.shipsToAdd);
			}
			break;
		case eMessageType::eStartOnlineGame :
			m_currentGameState = eGameState::eBattle;
			m_battle.startOnlineGame(receivedServerMessage.levelName, receivedServerMessage.spawnPositions);
			break;
		case eMessageType::eRefuseConnection :
			std::cout << "Connection Refused\n";
			m_window.close();
			break;
		case eMessageType::eDeployShipAtPosition :
		case eMessageType::eMoveShipToPosition :
		case eMessageType::eAttackShipAtPosition :
		case eMessageType::eClientDisconnected :
			if (getLocalControlledFaction() != receivedServerMessage.faction)
			{
				m_battle.receiveServerMessage(receivedServerMessage);
			}
			break;
		}
	}
}

void Game::handleInput()
{
	while (m_window.pollEvent(m_currentSFMLEvent))
	{
		sf::Vector2i mousePosition = sf::Mouse::getPosition(m_window);
		//mousePosition += MOUSE_POSITION_OFFSET;
		sf::IntRect mouseRect(mousePosition, sf::Vector2i(5, 5));
		mouseShape.setPosition(sf::Vector2f(mousePosition.x, mousePosition.y));

		switch (m_currentSFMLEvent.type)
		{
		case sf::Event::Closed :
			quit();
			break;

		case sf::Event::MouseButtonPressed :
			if (m_currentSFMLEvent.mouseButton.button == sf::Mouse::Left)
			{
				//Main Menu Input
				if (m_currentGameState == eGameState::eMainMenu)
				{
					for (const auto& textBox : m_mainMenuTextBoxes)
					{
						if (mouseRect.intersects(textBox.AABB))
						{
							switch (textBox.name)
							{
							case eUITextBoxName::ePlaySinglePlayer:
								m_currentGameState = eGameState::eSinglePlayerFactionSelect;

								break;

							case eUITextBoxName::ePlayerMultiplayer:
								break;

							case eUITextBoxName::eQuit:
								quit();
								break;
							}
						}
					}
				}
				//Single Player Faction Select
				else if (m_currentGameState == eGameState::eSinglePlayerFactionSelect)
				{
					for (auto& button : m_singlePlayerButtons)
					{
						if (mouseRect.intersects(button.AABB))
						{
							button.sprite.incrementFrameID();
						}
					}

					if (mouseRect.intersects(m_singlePlayerDoneButton.AABB))
					{

					}
				}
			}
			break;
		}

		//else if (m_currentSFMLEvent.type == sf::Event::KeyPressed)
		//{
		//	if (m_currentSFMLEvent.key.code == sf::Keyboard::R && m_onlineGame && !m_ready)
		//	{
		//		ServerMessage messageToSend(eMessageType::ePlayerReady, getLocalFactionName());
		//		NetworkHandler::getInstance().sendMessageToServer(messageToSend);
		//		m_ready = true;
		//	}
		//}
		//if (m_battle.isRunning() && (!m_onlineGame || (m_onlineGame && !m_gameLobbyActive)))
		//{
		//	m_battle.handleInput(m_window, m_currentSFMLEvent);
		//}
	}
}

void Game::render()
{
	m_window.clear(sf::Color::Black);

	switch (m_currentGameState)
	{
	case eGameState::eMainMenu :
		m_backgroundSprite.render(m_window);
		for (const auto& textBox : m_mainMenuTextBoxes)
		{
			m_window.draw(textBox.text);
		}

		break;
	case eGameState::eBattle :
		m_battle.render(m_window);
		
		break;

	case eGameState::eSinglePlayerFactionSelect :
		m_backgroundSprite.render(m_window);
		for (auto& button : m_singlePlayerButtons)
		{
			button.sprite.render(m_window);
		}
		break;
	}
	m_window.draw(mouseShape);

	m_window.display();
}

void Game::assignFaction(eFactionName factionName, eFactionControllerType controllerType, const std::vector<eShipType>& shipsToAdd)
{
	m_factions[static_cast<int>(factionName)].m_controllerType = controllerType;

	for (eShipType shipToAdd : shipsToAdd)
	{
		m_factions[static_cast<int>(factionName)].addShip(factionName, shipToAdd);
	}
}

eFactionName Game::getLocalControlledFaction() const
{
	auto cIter = std::find_if(m_factions.cbegin(), m_factions.cend(), [](const auto& faction)
	{ return faction.m_controllerType == eFactionControllerType::eLocalPlayer; });

	assert(cIter != m_factions.cend());
	return cIter->m_factionName;
}

void Game::quit()
{
	if (NetworkHandler::getInstance().isConnectedToServer())
	{
		NetworkHandler::getInstance().sendMessageToServer(ServerMessage(eMessageType::eDisconnect, getLocalControlledFaction()));
		NetworkHandler::getInstance().disconnectFromServer();
	}

	m_window.close();
	m_battle.quitGame();
}