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

Texture::Texture(const std::string& name, std::vector<FrameDetails>&& frames)
	: m_texture(),
	m_frames(std::move(frames))
{
	m_texture.loadFromFile(DATA_DIRECTORY + name);
}

const FrameDetails & Texture::getFrame(int frameID) const
{
	assert(frameID >= 0 && frameID < m_frames.size());
	return m_frames[frameID];
}