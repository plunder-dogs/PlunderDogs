#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "Utilities/NonCopyable.h"

struct FrameDetails
{
	FrameDetails(int height, int width, int y, int x, int ID);

	int height;
	int width;
	int y;
	int x;
	int ID;
};

struct Texture : NonCopyable
{
	Texture(const std::string& name, std::vector<FrameDetails>&& frames);
	
	const FrameDetails& getFrame(int frameID) const;

	sf::Texture m_texture;
	std::vector<FrameDetails> m_frames;
};