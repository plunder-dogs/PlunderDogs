#include "Textures.h"
#include "Battle.h"
#include "AI.h"
#include <array>
#include "SFML/Graphics.hpp"

//Copy Ellision
//Return Value Optimzation
//Stack Frames
//https://www.youtube.com/channel/UCSX3MR0gnKDxyXAyljWzm0Q/playlists

float getDeltaTime(int frameStart, int lastFrameStart)
{
	return static_cast<float>(frameStart - lastFrameStart) / 1000.0f;
}

int main()
{
	sf::Vector2u windowSize(1920, 1080);
	sf::RenderWindow window(sf::VideoMode(windowSize.x, windowSize.y), "SFML_WINDOW", sf::Style::Default);
	window.setFramerateLimit(60);

	Textures::loadAllTextures();

	std::array<std::unique_ptr<Faction>, static_cast<size_t>(FactionName::eTotal)> players;

	players[static_cast<int>(FactionName::eYellow)] = std::make_unique<Faction>(FactionName::eYellow, ePlayerType::eHuman);
	players[static_cast<int>(FactionName::eYellow)]->addShip(FactionName::eYellow, eShipType::eFrigate);
	players[static_cast<int>(FactionName::eYellow)]->addShip(FactionName::eYellow, eShipType::eFrigate);
	players[static_cast<int>(FactionName::eYellow)]->addShip(FactionName::eYellow, eShipType::eFrigate);
	players[static_cast<int>(FactionName::eYellow)]->addShip(FactionName::eYellow, eShipType::eFrigate);
	players[static_cast<int>(FactionName::eYellow)]->addShip(FactionName::eYellow, eShipType::eFrigate);
	players[static_cast<int>(FactionName::eYellow)]->addShip(FactionName::eYellow, eShipType::eFrigate);

	players[static_cast<int>(FactionName::eRed)] = std::make_unique<Faction>(FactionName::eRed, ePlayerType::eAI);
	AI::loadInPlayerShips(*players.back().get());

	Battle battle(players);

	battle.start("Level1.tmx");

	sf::Clock gameClock;
	float lastFrameStart = gameClock.restart().asMilliseconds();
	sf::Event currentEvent;
	while (window.isOpen())
	{
		float frameStart = gameClock.restart().asMilliseconds();
		
		while (window.pollEvent(currentEvent))
		{
			battle.handleInput(currentEvent);
		}

		battle.update(getDeltaTime(frameStart, lastFrameStart));

		window.clear();
		battle.render(window);
		window.display();

		lastFrameStart = frameStart;
	}

	return 0;
}