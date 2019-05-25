#include "Textures.h"
#include "Battle.h"

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
	
	std::vector<std::unique_ptr<Player>> players;

	std::unique_ptr<Player> player1 = std::make_unique<Player>(FactionName::eYellow, ePlayerType::eHuman);
	player1->m_ships.emplace_back(std::make_unique<Ship>(FactionName::eYellow, ShipType::eFrigate, 5, 5, 5, eWeaponType::eSideCannons));
	player1->m_ships.emplace_back(std::make_unique<Ship>(FactionName::eYellow, ShipType::eFrigate, 5, 5, 5, eWeaponType::eSideCannons));
	player1->m_ships.emplace_back(std::make_unique<Ship>(FactionName::eYellow, ShipType::eFrigate, 5, 5, 5, eWeaponType::eSideCannons));
	player1->m_ships.emplace_back(std::make_unique<Ship>(FactionName::eYellow, ShipType::eFrigate, 5, 5, 5, eWeaponType::eSideCannons));
	players.push_back(std::move(player1));

	std::unique_ptr<Player> player2 = std::make_unique<Player>(FactionName::eRed, ePlayerType::eHuman);
	player2->m_ships.emplace_back(std::make_unique<Ship>(FactionName::eYellow, ShipType::eFrigate, 5, 5, 5, eWeaponType::eSideCannons));
	player2->m_ships.emplace_back(std::make_unique<Ship>(FactionName::eYellow, ShipType::eFrigate, 5, 5, 5, eWeaponType::eSideCannons));
	player2->m_ships.emplace_back(std::make_unique<Ship>(FactionName::eYellow, ShipType::eFrigate, 5, 5, 5, eWeaponType::eSideCannons));
	player2->m_ships.emplace_back(std::make_unique<Ship>(FactionName::eYellow, ShipType::eFrigate, 5, 5, 5, eWeaponType::eSideCannons));
	players.push_back(std::move(player2));
	Battle battle(players);

	battle.start("Level1.tmx");

	while (HAPI_Sprites.Update())
	{
		int frameStart = HAPI_Sprites.GetTime();

		SCREEN_SURFACE->Clear();
		
		lastFrameStart = frameStart;
	}
}