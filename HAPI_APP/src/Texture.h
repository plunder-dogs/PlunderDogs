#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

struct FrameDetails
{
	FrameDetails(int height, int width, int y, int x, int ID);

	int height;
	int width;
	int y;
	int x;
	int ID;
};

struct Texture
{
	Texture(const std::string& name, std::vector<FrameDetails>&& frames);
	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;
	Texture(Texture&&) = delete;
	Texture&& operator=(Texture&&) = delete;
	
	const FrameDetails& getFrame(int frameID) const;

	sf::Texture m_texture;
	std::vector<FrameDetails> m_frames;
};