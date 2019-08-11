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

class Texture : private NonCopyable
{
public:	
	Texture() {}
	static std::unique_ptr<Texture> load(const std::string& fileName, std::vector<FrameDetails>&& frames);

	const FrameDetails& getFrame(int frameID) const;

	const sf::Texture& getTexture() const;
	const std::vector<FrameDetails>& getFrames() const;

private:
	
	sf::Texture m_texture;
	std::vector<FrameDetails> m_frames;
	bool m_textureSet = false;

	bool init(const std::string& fileName, std::vector<FrameDetails>&& frames);
};