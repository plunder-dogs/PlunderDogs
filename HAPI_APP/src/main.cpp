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

//https://www.youtube.com/watch?v=dn1XRIaROM4
//https://youtu.be/h47zZrqjgLc
//https://github.com/MFatihMAR/Awesome-Game-Networking

//Solution 1:
//Local host server

//Solution 2:
//current solution - "m_onlineGame" to tell if multiplayer or singleplayer

int main()
{
	if (!Textures::getInstance().loadAllTextures())
	{
		std::cerr << "Failed to load all textures.\n";
		return -1;
	}
	Game game;
	game.run();

	return 0;
}
