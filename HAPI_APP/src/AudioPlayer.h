#pragma once

#include <string>
#include <unordered_map>

//bool LoadSound(const std::string &filename)
//bool PlaySound(const std::string &filename)
//bool PlaySound(const std::string &filename, const SoundOptions &options)

//bool StopSound(int soundInstanceId, bool togglePause = false)
//bool ChangePlayingSoundOptions(int &soundInstanceId, const SoundOptions &options)
//const EMediaStatus GetSoundStatus(int soundInstanceId, float &playbackPosition) const

struct Sound
{
	Sound();
	Sound(int instanceId, const std::string& soundName, bool isplaying);
	int m_instanceId;
	std::string m_soundName;
	bool m_isPlaying;

};

class AudioPlayer
{
public:
	static AudioPlayer& getInstance()
	{
		static AudioPlayer instance;
		return instance;
	}
	
	void playSound(const std::string& soundName, float volume = 1, bool looping = false);
	
	void playShortSound(const std::string & soundName);
	void registerSound(const std::string& filename, const std::string& soundName);
	void stopSound(const std::string& soundName);


private:
	AudioPlayer();
	
	std::unordered_map < std::string, Sound > m_soundList;
	
	//std::deque<
};