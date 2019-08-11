#include "Texture.h"
#include "Global.h"
#include <utility>
#include <assert.h>

//Frame Details
FrameDetails::FrameDetails(int height, int width, int y, int x, int ID)
	: height(height),
	width(width),
	y(y),
	x(x),
	ID(ID)
{}

std::unique_ptr<Texture> Texture::load(const std::string & fileName, std::vector<FrameDetails>&& frames)
{
	std::unique_ptr<Texture> texture = std::make_unique<Texture>();
	if (texture->init(fileName, std::move(frames)))
	{
		return texture;
	}
	else
	{
		return std::unique_ptr<Texture>();
	}
}

const FrameDetails & Texture::getFrame(int frameID) const
{
	assert(frameID >= 0 && frameID < m_frames.size());
	return m_frames[frameID];
}

const sf::Texture & Texture::getTexture() const
{
	return m_texture;
}

const std::vector<FrameDetails>& Texture::getFrames() const
{
	return m_frames;
}

bool Texture::init(const std::string & fileName, std::vector<FrameDetails>&& frames)
{
	m_frames = std::move(frames);
	return m_texture.loadFromFile(fileName);
}