#include "Textures.h"
#include "Battle.h"
#include "AI.h"
#include <array>
#include "SFML/Graphics.hpp"
#include "Utilities/XMLParser.h"

#ifdef C++_NOTES
//Copy Ellision
//Return Value Optimzation
//Stack Frames
//https://www.youtube.com/channel/UCSX3MR0gnKDxyXAyljWzm0Q/playlists
//https://www.reddit.com/r/cpp/comments/byi5m3/books_on_memory_management_techniques/
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

int main()
{
	sf::Vector2u windowSize(1920, 1080);
	sf::RenderWindow window(sf::VideoMode(windowSize.x, windowSize.y), "SFML_WINDOW", sf::Style::Default);
	window.setFramerateLimit(120);
	
	Textures::getInstance().loadAllTextures();

	std::array<Faction, static_cast<size_t>(FactionName::eTotal)> players;

	players[static_cast<int>(FactionName::eYellow)].m_factionName = FactionName::eYellow;
	players[static_cast<int>(FactionName::eYellow)].m_playerType = ePlayerType::eHuman;
	players[static_cast<int>(FactionName::eYellow)].addShip(FactionName::eYellow, eShipType::eFrigate);
	players[static_cast<int>(FactionName::eYellow)].addShip(FactionName::eYellow, eShipType::eFrigate);
	players[static_cast<int>(FactionName::eYellow)].addShip(FactionName::eYellow, eShipType::eFrigate);
	players[static_cast<int>(FactionName::eYellow)].addShip(FactionName::eYellow, eShipType::eFrigate);
	players[static_cast<int>(FactionName::eYellow)].addShip(FactionName::eYellow, eShipType::eFrigate);
	players[static_cast<int>(FactionName::eYellow)].addShip(FactionName::eYellow, eShipType::eFrigate);

	players[static_cast<int>(FactionName::eRed)].m_factionName = FactionName::eRed;
	players[static_cast<int>(FactionName::eRed)].m_playerType = ePlayerType::eAI;
	AI::loadShips(players[static_cast<int>(FactionName::eRed)]);

	Battle battle(players);
	battle.start("Level1.tmx");

	sf::Clock gameClock;
	sf::Event currentEvent;
	float deltaTime = gameClock.restart().asSeconds();
	while (battle.isRunning())
	{
		while (window.pollEvent(currentEvent))
		{
			if (currentEvent.type == sf::Event::Closed)
			{
				window.close();
			}

			battle.handleInput(window, currentEvent);
		}
		
		battle.update(deltaTime);

		window.clear();
		battle.render(window);
		window.display();

		deltaTime = gameClock.restart().asSeconds();
	}

	return 0;
}