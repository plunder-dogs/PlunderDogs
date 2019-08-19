#include "Game.h"
#include "GameEventMessenger.h"
#include "AIHandler.h"
#include "Sprite.h"
#include "Texture.h"
#include <iostream>

Game::Game(const sf::Font& font)
	: m_window(sf::VideoMode(1920, 1080), "SFML_WINDOW", sf::Style::Default),
	m_currentGameState(eGameState::eMainMenu),
	m_ready(false),
	m_currentSFMLEvent(),
	m_gameClock(),
	m_deltaTime(m_gameClock.restart().asSeconds())
{
	//Main Menu - UI
	std::vector<UIComponentTextBox> mainMenuTextBoxes;
	mainMenuTextBoxes.reserve(size_t(3));
	mainMenuTextBoxes.emplace_back("Play Singleplayer", font, sf::Vector2i(600, 600), eUIComponentName::ePlaySinglePlayer);
	mainMenuTextBoxes.emplace_back("Play Multiplayer", font, sf::Vector2i(600, 680), eUIComponentName::ePlayerMultiplayer);
	mainMenuTextBoxes.emplace_back("Quit", font, sf::Vector2i(600, 760), eUIComponentName::eQuit);
	mainMenuTextBoxes.emplace_back("Cannot connect to MultiPlayer", font, sf::Vector2i(960, 540), eUIComponentName::eAlert, false);
	m_UILayers[static_cast<int>(eGameState::eMainMenu)].setTextBoxes(std::move(mainMenuTextBoxes));
	std::vector<UIComponentImage> mainMenuSprites;
	mainMenuSprites.emplace_back(Textures::getInstance().getTexture(BACKGROUND), sf::Vector2i(), eUIComponentName::MainMenuBackground, true);
	m_UILayers[static_cast<int>(eGameState::eMainMenu)].setImages(std::move(mainMenuSprites));

	//Single Player Faction Select - UI
	std::vector<UIComponentButton> singlePlayerFactionSelectButtons;
	singlePlayerFactionSelectButtons.reserve(size_t(6));
	singlePlayerFactionSelectButtons.emplace_back(Textures::getInstance().getTexture("yellowSelectBtn.xml"), sf::Vector2i(400, 250), eUIComponentName::eYellowFactionSelect);
	singlePlayerFactionSelectButtons.emplace_back(Textures::getInstance().getTexture("blueSelectBtn.xml"), sf::Vector2i(600, 250), eUIComponentName::eBlueFactionSelect);
	singlePlayerFactionSelectButtons.emplace_back(Textures::getInstance().getTexture("greenSelectBtn.xml"), sf::Vector2i(800, 250), eUIComponentName::eGreenFactionSelect);
	singlePlayerFactionSelectButtons.emplace_back(Textures::getInstance().getTexture("redSelectBtn.xml"), sf::Vector2i(1000, 250), eUIComponentName::eRedFactionSelect);
	singlePlayerFactionSelectButtons.emplace_back(Textures::getInstance().getTexture("backButton.xml"), sf::Vector2i(250, 700), eUIComponentName::eBack, true);
	singlePlayerFactionSelectButtons.emplace_back(Textures::getInstance().getTexture("doneButton.xml"), sf::Vector2i(800, 800), eUIComponentName::eDone, true);
	m_UILayers[static_cast<int>(eGameState::eSinglePlayerFactionSelect)].setButtons(std::move(singlePlayerFactionSelectButtons));
	std::vector<UIComponentImage> singlePlayerFactionSelectImages;
	singlePlayerFactionSelectImages.emplace_back(Textures::getInstance().getTexture("GameBackGround.xml"), sf::Vector2i(), eUIComponentName::eGameBackground, true);
	m_UILayers[static_cast<int>(eGameState::eSinglePlayerFactionSelect)].setImages(std::move(singlePlayerFactionSelectImages));

	//Level Selection - UI
	std::vector<UIComponentButton> levelSelectButtons;
	levelSelectButtons.reserve(size_t(6));
	levelSelectButtons.emplace_back(Textures::getInstance().getTexture("level1Button.xml"), sf::Vector2i(200, 200), eUIComponentName::eLevelOneSelect, true);
	levelSelectButtons.emplace_back(Textures::getInstance().getTexture("level2Button.xml"), sf::Vector2i(1600, 200), eUIComponentName::eLevelTwoSelect, true);
	levelSelectButtons.emplace_back(Textures::getInstance().getTexture("level3Button.xml"), sf::Vector2i(200, 700), eUIComponentName::eLevelThreeSelect, true);
	levelSelectButtons.emplace_back(Textures::getInstance().getTexture("level4Button.xml"), sf::Vector2i(1600, 700), eUIComponentName::eLevelFourSelect, true);
	levelSelectButtons.emplace_back(Textures::getInstance().getTexture("level5Button.xml"), sf::Vector2i(700, 700), eUIComponentName::eLevelFiveSelect, true);
	levelSelectButtons.emplace_back(Textures::getInstance().getTexture("backButton.xml"), sf::Vector2i(200, 900), eUIComponentName::eBack, true);
	m_UILayers[static_cast<int>(eGameState::eLevelSelection)].setButtons(std::move(levelSelectButtons));
	std::vector<UIComponentImage> levelSelectImages;
	levelSelectImages.emplace_back(Textures::getInstance().getTexture("GameBackGround.xml"), sf::Vector2i(), eUIComponentName::eGameBackground, true);
	m_UILayers[static_cast<int>(eGameState::eLevelSelection)].setImages(std::move(levelSelectImages));

	//Battle - UI
	std::vector<UIComponentButton> battleButtons;
	battleButtons.emplace_back(Textures::getInstance().getTexture("EndPhaseButtons.xml"), sf::Vector2i(200, 800), eUIComponentName::eEndPhase, true, false);
	battleButtons.emplace_back(Textures::getInstance().getTexture("pauseButton.xml"), sf::Vector2i(1400, 200), eUIComponentName::ePause, true);
	m_UILayers[static_cast<int>(eGameState::eBattle)].setButtons(std::move(battleButtons));
	std::vector<UIComponentImage> battleImages;
	battleImages.emplace_back(Textures::getInstance().getTexture("playerFlags.xml"), sf::Vector2i(500, 100), eUIComponentName::eFactionFlags);
	m_UILayers[static_cast<int>(eGameState::eBattle)].setImages(std::move(battleImages));

	m_window.setFramerateLimit(120);

	m_mouseShape.setFillColor(sf::Color::Red);
	m_mouseShape.setSize(sf::Vector2f(25, 25));

	//Subscribe to events
	GameEventMessenger::getInstance().subscribe(std::bind(&Game::onAllFactionsFinishedDeployment, this, std::placeholders::_1), eGameEvent::eAllFactionsFinishedDeployment);
	GameEventMessenger::getInstance().subscribe(std::bind(&Game::onNewFactionTurn, this, std::placeholders::_1), eGameEvent::eEnteredNewFactionTurn);
	GameEventMessenger::getInstance().subscribe(std::bind(&Game::onEnteredAITurn, this, std::placeholders::_1), eGameEvent::eEnteredAITurn);
	GameEventMessenger::getInstance().subscribe(std::bind(&Game::onLeftAITurn, this, std::placeholders::_1), eGameEvent::eLeftAITurn);
}

Game::~Game()
{
	GameEventMessenger::getInstance().unsubscribe(eGameEvent::eAllFactionsFinishedDeployment);
	GameEventMessenger::getInstance().unsubscribe(eGameEvent::eEnteredNewFactionTurn);
	GameEventMessenger::getInstance().unsubscribe(eGameEvent::eEnteredAITurn);
	GameEventMessenger::getInstance().unsubscribe(eGameEvent::eLeftAITurn);
}

void Game::run()
{
	while (m_window.isOpen())
	{
		handleServerMessages();
		handleInput();
		m_UILayers[static_cast<int>(m_currentGameState)].update(m_deltaTime);
		if (m_currentGameState == eGameState::eBattle)
		{
			assert(m_battle);
			m_battle->update(m_deltaTime);
		}

		render();
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
			m_battle = Battle::startOnlineGame(m_factions, receivedServerMessage.levelName, receivedServerMessage.spawnPositions);
			if (!m_battle)
			{
				//Battle failed to load
				m_currentGameState = eGameState::eMainMenu;
			}
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
				assert(m_battle);
				m_battle->receiveServerMessage(receivedServerMessage);
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
		m_mouseShape.setPosition(sf::Vector2f(mousePosition.x, mousePosition.y));

		switch (m_currentSFMLEvent.type)
		{
		case sf::Event::Closed :
			quit();
			break;

		case sf::Event::MouseMoved :
			m_UILayers[static_cast<int>(m_currentGameState)].onComponentIntersect(mouseRect);
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
				case eGameState::eLevelSelection :
					handleLevelSelectionInput(mouseRect);
					break;
				case eGameState::eBattle :
					handleBattleInput(mouseRect);
					break;
				}
			}
			break;
		}

		if (m_currentGameState == eGameState::eBattle)
		{
			assert(m_battle);
			m_battle->handleInput(m_window, m_currentSFMLEvent);
		}
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
			if (NetworkHandler::getInstance().connectToServer())
			{
				m_currentGameState = eGameState::eMultiPlayerFactionSelect;
			}
			else
			{
				m_UILayers[static_cast<int>(m_currentGameState)].activateTimedVisibilityComponent(eUIComponentName::eAlert, eUIComponentType::eTextBox);
			}
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
		case eUIComponentName::eDone :
		{
			const UILayer& currentUILayer = m_UILayers[static_cast<int>(m_currentGameState)];
			fillFaction(eFactionName::eRed, currentUILayer.getButton(eUIComponentName::eRedFactionSelect).sprite.getCurrentFrameID());
			fillFaction(eFactionName::eYellow, currentUILayer.getButton(eUIComponentName::eYellowFactionSelect).sprite.getCurrentFrameID());
			fillFaction(eFactionName::eGreen, currentUILayer.getButton(eUIComponentName::eGreenFactionSelect).sprite.getCurrentFrameID());
			fillFaction(eFactionName::eBlue, currentUILayer.getButton(eUIComponentName::eBlueFactionSelect).sprite.getCurrentFrameID());

			m_currentGameState = eGameState::eLevelSelection;
			break;
		}
		case eUIComponentName::eBack :
		{
			m_UILayers[static_cast<int>(m_currentGameState)].resetButtons();

			m_currentGameState = eGameState::eMainMenu;
			break;
		}
		}
	}
}

void Game::handleLevelSelectionInput(sf::IntRect mouseRect)
{
	UIComponentIntersectionDetails intersectionDetails;
	m_UILayers[static_cast<int>(m_currentGameState)].onComponentIntersect(mouseRect, intersectionDetails);
	if (intersectionDetails.isIntersected())
	{
		switch (intersectionDetails.getComponentName())
		{
		case eUIComponentName::eLevelOneSelect :
			m_currentGameState = eGameState::eBattle;
			m_battle = Battle::startSinglePlayerGame(m_factions, "level1.tmx");
			break;

		case eUIComponentName::eLevelTwoSelect :
			m_currentGameState = eGameState::eBattle;
			m_battle = Battle::startSinglePlayerGame(m_factions, "level2.tmx");
			break;
		
		case eUIComponentName::eLevelThreeSelect :
			m_currentGameState = eGameState::eBattle;
			m_battle = Battle::startSinglePlayerGame(m_factions, "level3.tmx");
			break;
		
		case eUIComponentName::eLevelFourSelect :
			m_currentGameState = eGameState::eBattle;
			m_battle = Battle::startSinglePlayerGame(m_factions, "level4.tmx");
			break;
		
		case eUIComponentName::eLevelFiveSelect :
			m_currentGameState = eGameState::eBattle;
			m_battle = Battle::startSinglePlayerGame(m_factions, "level5.tmx");
			break;

		case eUIComponentName::eBack :
			m_currentGameState = eGameState::eMainMenu;
			break;
		}

		//Battle failed to load
		if (m_currentGameState == eGameState::eBattle && !m_battle)
		{
			m_currentGameState = eGameState::eMainMenu;
		}
	}
}

void Game::handleBattleInput(sf::IntRect mouseRect)
{
	UIComponentIntersectionDetails intersectionDetails;
	m_UILayers[static_cast<int>(m_currentGameState)].onComponentIntersect(mouseRect, intersectionDetails);
	if (intersectionDetails.isIntersected())
	{
		assert(m_battle);
		switch (intersectionDetails.getComponentName())
		{
		case eUIComponentName::eEndPhase :
			m_battle->endCurrentBattlePhase();
			break;
		case eUIComponentName::ePause :
			m_battle.reset();
			resetAllFactions();
			m_currentGameState = eGameState::eMainMenu;
			break;
		}
	}
}

void Game::render()
{
	m_window.clear(sf::Color::Black);
	if(m_currentGameState == eGameState::eBattle)
	{
		assert(m_battle);
		m_battle->render(m_window);
	}
	m_UILayers[static_cast<int>(m_currentGameState)].render(m_window);
	m_window.draw(m_mouseShape);

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

void Game::fillFaction(eFactionName factionName, int frameID)
{
	std::vector<eShipType> shipsToAdd;
	shipsToAdd.reserve(size_t(6));
	shipsToAdd.push_back(eShipType::eFrigate);
	shipsToAdd.push_back(eShipType::eFrigate);
	shipsToAdd.push_back(eShipType::eFrigate);
	shipsToAdd.push_back(eShipType::eFrigate);
	shipsToAdd.push_back(eShipType::eFrigate);
	shipsToAdd.push_back(eShipType::eFrigate);

	eFactionControllerType& factionControllerType = m_factions[static_cast<int>(factionName)].m_controllerType;
	switch (frameID)
	{
	case 0:
		factionControllerType = eFactionControllerType::None;
		break;
	case 1:
		m_factions[static_cast<int>(factionName)].m_controllerType = eFactionControllerType::eLocalPlayer;
		for (eShipType shipToAdd : shipsToAdd)
		{
			m_factions[static_cast<int>(factionName)].addShip(factionName, shipToAdd);
		}
		break;
	case 2:
		m_factions[static_cast<int>(factionName)].m_controllerType = eFactionControllerType::eAI;
		for (eShipType shipToAdd : shipsToAdd)
		{
			m_factions[static_cast<int>(factionName)].addShip(factionName, shipToAdd);
		}
		break;
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
}

void Game::resetAllFactions()
{
	for (auto& faction : m_factions)
	{
		faction.m_controllerType = eFactionControllerType::None;
		faction.m_ships.clear();
		faction.m_spawnArea.clearTileArea();
	}
}

void Game::onAllFactionsFinishedDeployment(GameEvent gameEvent)
{
	assert(m_battle && m_battle->getCurrentBattlePhase() != eBattlePhase::Deployment);
	m_UILayers[static_cast<int>(m_currentGameState)].setComponentVisibility(eUIComponentName::eEndPhase, eUIComponentType::eButton, true);
}

void Game::onNewFactionTurn(GameEvent gameEvent)
{
	assert(m_currentGameState == eGameState::eBattle);
	switch (m_battle->getCurrentFaction().m_factionName)
	{
	case eFactionName::eYellow :
		m_UILayers[static_cast<int>(m_currentGameState)].setComponentFrameID(eUIComponentName::eFactionFlags, eUIComponentType::eImage, 0);
		break;
	case eFactionName::eBlue :
		m_UILayers[static_cast<int>(m_currentGameState)].setComponentFrameID(eUIComponentName::eFactionFlags, eUIComponentType::eImage, 1);
		break;
	case eFactionName::eGreen :
		m_UILayers[static_cast<int>(m_currentGameState)].setComponentFrameID(eUIComponentName::eFactionFlags, eUIComponentType::eImage, 2);
		break;
	case eFactionName::eRed :
		m_UILayers[static_cast<int>(m_currentGameState)].setComponentFrameID(eUIComponentName::eFactionFlags, eUIComponentType::eImage, 3);
		break;
	}
}

void Game::onEnteredAITurn(GameEvent gameEvent)
{
	assert(m_battle && m_battle->getCurrentFaction().m_controllerType == eFactionControllerType::eAI);
	m_UILayers[static_cast<int>(m_currentGameState)].setComponentVisibility(eUIComponentName::eEndPhase, eUIComponentType::eButton, false);
}

void Game::onLeftAITurn(GameEvent gameEvent)
{
	assert(m_battle && m_battle->getCurrentFaction().m_controllerType != eFactionControllerType::eAI);
	m_UILayers[static_cast<int>(m_currentGameState)].setComponentVisibility(eUIComponentName::eEndPhase, eUIComponentType::eButton, true);
}