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
	Player player1(FactionName::eYellow, ePlayerType::eAI);
	player1.m_ships.emplace_back()



	while (HAPI_Sprites.Update())
	{
		int frameStart = HAPI_Sprites.GetTime();

		SCREEN_SURFACE->Clear();
		
		lastFrameStart = frameStart;
	}
}