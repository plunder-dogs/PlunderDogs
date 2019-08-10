#include "Texture.h"
#include "Global.h"
#include <utility>
#include <assert.h>

FrameDetails::FrameDetails(int height, int width, int y, int x, int ID)
	: height(height),
	width(width),
	y(y),
	x(x),
	ID(ID)
{}

Texture::Texture(std::vector<FrameDetails>&& frames, sf::Texture&& texture)
	: m_texture(std::move(texture)),
	m_frames(std::move(frames))
{}

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
