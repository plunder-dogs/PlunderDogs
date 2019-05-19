#include "AudioPlayer.h"
#include <HAPISprites_lib.h>
#include <HAPISprites_UI.h>
#include "Utilities/Utilities.h"
#include <iostream>


AudioPlayer::AudioPlayer() :m_soundList()
{
	
}

void AudioPlayer::playSound(const std::string & soundName,  float volume , bool looping)
{
	std::cout << "play Sound\n";
	if (!m_soundList[soundName].m_isPlaying)
	{
		HAPI_Sprites.PlayStreamedMedia(Utilities::getDataDirectory() + m_soundList[soundName].m_soundName,
			HAPISPACE::SoundOptions::HAPI_TSoundOptions(volume, looping	), m_soundList[soundName].m_instanceId);
		

		m_soundList[soundName].m_isPlaying = true;
	}

}


void AudioPlayer::playShortSound(const std::string & soundName)
{
	std::cout << "play Sound\n";
	if (!m_soundList[soundName].m_isPlaying)
	{
		HAPI_Sprites.PlayStreamedMedia(Utilities::getDataDirectory() + m_soundList[soundName].m_soundName,
			HAPISPACE::SoundOptions::HAPI_TSoundOptions(0.6, false), m_soundList[soundName].m_instanceId);

	}

}


void AudioPlayer::registerSound(const std::string & filename,  const std::string & soundName)
{
	Sound sound(0, filename, false);
	m_soundList[soundName] = sound;
	
}

void AudioPlayer::stopSound(const std::string & soundName)
{
	if (m_soundList[soundName].m_isPlaying)
	{
		
		HAPI_Sprites.StopStreamedMedia(m_soundList[soundName].m_instanceId);
		m_soundList[soundName].m_isPlaying = false;
	}
	
}


Sound::Sound()
{
}

Sound::Sound(int instanceId, const std::string& soundName, bool isplaying):
	m_instanceId(instanceId),
	m_soundName(soundName), 
	m_isPlaying(isplaying)
{
}
