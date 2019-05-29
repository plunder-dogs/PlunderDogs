#include "Textures.h"
#include "Battle.h"
#include "AI.h"
#include <array>

using namespace HAPISPACE;

float getDeltaTime(int frameStart, int lastFrameStart)
{
	return static_cast<float>(frameStart - lastFrameStart) / 1000.0f;
}

void HAPI_Sprites_Main()
{
	std::pair<int, int> windowSize(1920, 1080);
	if (!HAPI_Sprites.Initialise(windowSize.first, windowSize.second, "Plunder Dogs", eHSEnableUI))
		return;

	if (!Textures::loadAllTextures())
	{
		HAPI_Sprites.UserMessage("Could not pre-load all textures", "Texture Error");
		return;
	}

	HAPI_Sprites.SetShowFPS(true);
	HAPI_Sprites.LimitFrameRate(150);
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