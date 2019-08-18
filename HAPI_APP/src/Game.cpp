#include "Game.h"
#include "GameEventMessenger.h"
#include "Sprite.h"
#include "Texture.h"
#include <iostream>

Game::Game(const sf::Font& font)
	: m_window(sf::VideoMode(1920, 1080), "SFML_WINDOW", sf::Style::Default),
	m_currentGameState(eGameState::eMainMenu),
	m_ready(false),
	m_battle(m_factions),
	m_currentSFMLEvent(),
	m_gameClock(),
	m_deltaTime(m_gameClock.restart().asSeconds())
{
	//Main Menu
	std::vector<UIComponentTextBox> mainMenuTextBoxes;
	mainMenuTextBoxes.reserve(size_t(3));
	mainMenuTextBoxes.emplace_back("Play Singleplayer", font, sf::Vector2i(600, 600), eUIComponentName::ePlaySinglePlayer);
	mainMenuTextBoxes.emplace_back("Play Multiplayer", font, sf::Vector2i(600, 680), eUIComponentName::ePlayerMultiplayer);
	mainMenuTextBoxes.emplace_back("Quit", font, sf::Vector2i(600, 760), eUIComponentName::eQuit);
	m_UILayers[static_cast<int>(eGameState::eMainMenu)].setTextBoxes(std::move(mainMenuTextBoxes));
	std::vector<Sprite> mainMenuSprites;
	mainMenuSprites.emplace_back(Textures::getInstance().getTexture(BACKGROUND), true, false);
	m_UILayers[static_cast<int>(eGameState::eMainMenu)].setImages(std::move(mainMenuSprites));
	

	//Single Player Faction Select
	std::vector<UIComponentButton> singlePlayerFactionSelectButtons;
	singlePlayerFactionSelectButtons.reserve(size_t(4));
	singlePlayerFactionSelectButtons.emplace_back(Textures::getInstance().getTexture("blueSelectBtn.xml"), sf::Vector2i(400, 250), eUIComponentName::eBlueFactionSelect);
	singlePlayerFactionSelectButtons.emplace_back(Textures::getInstance().getTexture("greenSelectBtn.xml"), sf::Vector2i(600, 250), eUIComponentName::eGreenFactionSelect);
	singlePlayerFactionSelectButtons.emplace_back(Textures::getInstance().getTexture("redSelectBtn.xml"), sf::Vector2i(800, 250), eUIComponentName::eRedFactionSelect);
	singlePlayerFactionSelectButtons.emplace_back(Textures::getInstance().getTexture("yellowSelectBtn.xml"), sf::Vector2i(1000, 250), eUIComponentName::eYellowFactionSelect);
	singlePlayerFactionSelectButtons.emplace_back(Textures::getInstance().getTexture("backButton.xml"), sf::Vector2i(250, 700), eUIComponentName::eBack, true);
	m_UILayers[static_cast<int>(eGameState::eSinglePlayerFactionSelect)].setButtons(std::move(singlePlayerFactionSelectButtons));
	std::vector<Sprite> singlePlayerFactionSelectImages;
	singlePlayerFactionSelectImages.emplace_back(Textures::getInstance().getTexture(BACKGROUND), true, false);
	m_UILayers[static_cast<int>(eGameState::eSinglePlayerFactionSelect)].setImages(std::move(singlePlayerFactionSelectImages));

	//Level Select
	std::vector<UIComponentButton> singlePlayerFactionSelectButtons;


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
		sf::IntRect mouseRect(mousePosition, sf::Vector2i(5, 5));
		mouseShape.setPosition(sf::Vector2f(mousePosition.x, mousePosition.y));

		switch (m_currentSFMLEvent.type)
		{
		case sf::Event::Closed :
			quit();
			break;

		case sf::Event::MouseMoved :
			m_UILayers[static_cast<int>(eGameState::eSinglePlayerFactionSelect)].onComponentIntersect(mouseRect);
			break;
		
		case sf::Event::MouseButtonPressed :
			if (m_currentSFMLEvent.mouseButton.button == sf::Mouse::Left)
			{
				switch (m_currentGameState)
				{
				case eGameState::eMainMenu :
					handleMainMenuInput(mouseRect);
					break;
				case eGameState::eSinglePlayerFactionSelect :
					handleSinglePlayerFactionSelectionInput(mouseRect);
					break;
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

void Game::handleMainMenuInput(sf::IntRect mouseRect)
{
	UIComponentIntersectionDetails intersectionDetails;
	m_UILayers[static_cast<int>(m_currentGameState)].onComponentIntersect(mouseRect, intersectionDetails);
	if (intersectionDetails.isIntersected())
	{
		switch (intersectionDetails.getComponentName())
		{
		case eUIComponentName::ePlaySinglePlayer :
			m_currentGameState = eGameState::eSinglePlayerFactionSelect;
			break;

		case eUIComponentName::ePlayerMultiplayer :
			m_currentGameState = eGameState::eMultiPlayerFactionSelect;
			break;

		case eUIComponentName::eQuit :
			quit();
			break;
		}
	}
}

void Game::handleSinglePlayerFactionSelectionInput(sf::IntRect mouseRect)
{
	UIComponentIntersectionDetails intersectionDetails;
	m_UILayers[static_cast<int>(m_currentGameState)].onComponentIntersect(mouseRect, intersectionDetails);
	if (intersectionDetails.isIntersected())
	{
		switch (intersectionDetails.getComponentName())
		{
		case eUIComponentName::eBack :
			m_UILayers[static_cast<int>(m_currentGameState)].resetButtonsFrameID();
			m_currentGameState = eGameState::eMainMenu;
			break;
		}
	}
}

void Game::render()
{
	m_window.clear(sf::Color::Black);

	m_UILayers[static_cast<int>(m_currentGameState)].render(m_window);

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
	auto cIter = std::find_if(m_factions.cbegin(), m_factions.cend(), [] (const auto& faction)
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