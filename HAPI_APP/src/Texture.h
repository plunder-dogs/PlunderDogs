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

class Texture : NonCopyable
{
public:
	Texture(std::vector<FrameDetails>&& frames, sf::Texture&& texture);
	
	const FrameDetails& getFrame(int frameID) const;

	const sf::Texture& getTexture() const;
	const std::vector<FrameDetails>& getFrames() const;

private:
	sf::Texture m_texture;
	std::vector<FrameDetails> m_frames;
};