#include "Game.h"

#ifdef C++_NOTES
//Copy Ellision
//Return Value Optimzation
//Stack Frames
//https://www.youtube.com/channel/UCSX3MR0gnKDxyXAyljWzm0Q/playlists
//https://www.reddit.com/r/cpp/comments/byi5m3/books_on_memory_management_techniques/
//https://stackoverflow.com/questions/37960797/threads-termination-of-infinite-loop-thread-in-c
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
	std::cout << "Select Game Mode: \n";
	std::cout << "Single Player: '1'\n";
	std::cout << "Multiplayer: '2'\n";
	int gameModeInput = 0;
	bool onlineGame = false;
	while (gameModeInput != 1 && gameModeInput != 2)
	{
		std::cin >> gameModeInput;
		if (gameModeInput == 2)
		{
			if (!NetworkHandler::getInstance().connect())
			{
				return -1;
			}

			onlineGame = true;
		}
	}
	Textures::getInstance().loadAllTextures();
	Game game(onlineGame);
	game.run();

	return 0;
}
