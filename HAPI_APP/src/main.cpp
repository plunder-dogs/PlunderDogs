#include "Textures.h"
#include "Battle.h"
#include "AI.h"
#include <array>
#include "SFML/Graphics.hpp"

using namespace HAPISPACE;

float getDeltaTime(int frameStart, int lastFrameStart)
{
	return static_cast<float>(frameStart - lastFrameStart) / 1000.0f;
}

void HAPI_Sprites_Main()
{
	sf::Vector2u windowSize(1920, 1080);
	sf::RenderWindow window(sf::VideoMode(windowSize.x, windowSize.y), "SFML_WINDOW", sf::Style::Default);
	window.setFramerateLimit(60);

	if (!Textures::loadAllTextures())
	{
		return;
	}

	int lastFrameStart = HAPI_Sprites.GetTime();
	
	std::array<std::unique_ptr<Faction>, static_cast<size_t>(FactionName::MAX)> players;

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

	while (HAPI_Sprites.Update())
	{
		int frameStart = HAPI_Sprites.GetTime();

		SCREEN_SURFACE->Clear();

		battle.update(getDeltaTime(frameStart, lastFrameStart));
		battle.render();
		
		lastFrameStart = frameStart;
	}
}