#pragma once

#include "Textures.h"
#include "Battle.h"
#include "AIHandler.h"
#include <array>
#include "SFML/Graphics.hpp"
#include "Utilities/XMLParser.h"
#include <iostream>
#include "NetworkHandler.h"
#include <assert.h>

enum class eUIButtonName
{
	eYellowFactionSelect = 0,
	eBlueFactionSelect,
	eGreenFactionSelect,
	eRedFactionSelect
};

struct ButtonInteraction
{
	bool intersected;
	int frameID;
};

enum class eUITextBoxName
{
	ePlaySinglePlayer = 0,
	ePlayerMultiplayer,
	eQuit
};

struct UIButton
{
	UIButton(const Texture& texture, sf::Vector2i position, eUIButtonName name)
		: name(name),
		sprite(texture, position, true, false),
		AABB(position, sf::Vector2i(sprite.getSize().x, sprite.getSize().y))
	{}

	const eUIButtonName name;
	Sprite sprite;
	sf::IntRect AABB;
};

struct TextBox
{
	TextBox(const std::string& message, const sf::Font& font, sf::Vector2f position, eUITextBoxName textBoxName)
		: name(textBoxName),
		text(message, font),
		AABB(sf::Vector2i(position.x - 5, position.y - 5), sf::Vector2i(text.getLocalBounds().width + 5, text.getLocalBounds().height + 5))
	{
		text.setPosition(position);
	}

	const eUITextBoxName name;
	sf::Text text;
	sf::IntRect AABB;
};



class Game : private NonCopyable 
{
public:
	Game(const sf::Font& font);

	void run();

private:
	std::array<Faction, static_cast<size_t>(eFactionName::eTotal)> m_factions
	{
		eFactionName::eYellow, 
		eFactionName::eBlue, 
		eFactionName::eGreen, 
		eFactionName::eRed
	};
	sf::RenderWindow m_window;
	eGameState m_currentGameState;
	bool m_ready;
	Battle m_battle;
	sf::Event m_currentSFMLEvent;
	sf::Clock m_gameClock;
	float m_deltaTime;
	
	//UI
	sf::RectangleShape mouseShape;
	const sf::Font& m_font;
	//Main Menu
	Sprite m_backgroundSprite;
	std::vector<TextBox> m_mainMenuTextBoxes;
	//Single Player Faction Select
	std::vector<UIButton> m_singlePlayerButtons;
	UIButton m_singlePlayerDoneButton;

	void handleServerMessages();
	void handleInput();
	void render();
	void assignFaction(eFactionName factionName, eFactionControllerType controllerType, const std::vector<eShipType>& shipsToAdd);
	eFactionName getLocalControlledFaction() const;

	void quit();
};