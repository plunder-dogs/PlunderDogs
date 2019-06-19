#include "Sprite.h"
#include "Texture.h"
#include <assert.h>
#include "Global.h"
#include "Map.h"

Sprite::Sprite(bool active)
	: m_texture(nullptr),
	m_sprite(),
	m_position(),
	m_currentFrameID(0),
	m_isActive(active)
{
	setOriginAtCenter();
}

Sprite::Sprite(const Texture & texture, sf::Vector2i startingPosition, bool active)
	: m_texture(&texture),
	m_sprite(),
	m_position(startingPosition),
	m_currentFrameID(0),
	m_isActive(active) 
{
	m_sprite.setTexture(m_texture->m_texture);
	setFrameID(m_currentFrameID);
	setOriginAtCenter();
}

Sprite::Sprite(std::unique_ptr<Texture>& texture, bool active)
	: m_texture(texture.get()),
	m_sprite(),
	m_position(),
	m_currentFrameID(0),
	m_isActive(active)
{
	m_sprite.setTexture(m_texture->m_texture);
	setFrameID(m_currentFrameID);
	setOriginAtCenter();
}

Sprite::Sprite(std::unique_ptr<Texture>& texture, sf::Vector2f startingPosition, bool active)
	: m_texture(texture.get()),
	m_sprite(),
	m_position(startingPosition),
	m_currentFrameID(0),
	m_isActive(active)
{
	m_sprite.setTexture(texture->m_texture);
	setFrameID(m_currentFrameID);
	setOriginAtCenter();
}

sf::Vector2f Sprite::getSize() const
{
	return sf::Vector2f(m_sprite.getLocalBounds().width, m_sprite.getLocalBounds().height);
}

sf::Vector2i Sprite::getPosition() const
{
	return m_position;
}

const FrameDetails & Sprite::getCurrentFrameDetails() const
{
	assert(m_texture);
	return m_texture->getFrame(m_currentFrameID);
}

bool Sprite::isActive() const
{
	return m_isActive;
}

bool Sprite::isAnimationCompleted() const
{
	return m_currentFrameID >= static_cast<int>(m_texture->m_frames.size()) - 1;
}

int Sprite::getCurrentFrameID() const
{
	return m_currentFrameID;
}

void Sprite::setFrameID(int frameID)
{
	assert(m_texture);

	m_currentFrameID = frameID;
	FrameDetails frame = m_texture->getFrame(m_currentFrameID);
	sf::IntRect frameRect(sf::Vector2i(frame.x, frame.y), sf::Vector2i(frame.width, frame.height));
	m_sprite.setTextureRect(frameRect);
}

void Sprite::incrementFrameID()
{
	assert(m_texture);
	assert(m_currentFrameID < static_cast<int>(m_texture->m_frames.size()) - 1);
	if (m_currentFrameID + 1 < static_cast<int>(m_texture->m_frames.size()))
	{
		++m_currentFrameID;
	}

	FrameDetails frame = m_texture->getFrame(m_currentFrameID);
	sf::IntRect frameRect(sf::Vector2i(frame.x, frame.y), sf::Vector2i(frame.width, frame.height));
	m_sprite.setTextureRect(frameRect);
}

void Sprite::setPosition(sf::Vector2i newPosition)
{
	m_sprite.setPosition(sf::Vector2f((float)newPosition.x, (float)newPosition.y));
	m_position = newPosition;
}

void Sprite::setRotation(float angle)
{
	m_sprite.setRotation(angle);
}

void Sprite::rotate(float angle)
{
	m_sprite.setRotation(m_sprite.getRotation() + angle);
}

void Sprite::setOriginAtCenter()
{
	sf::FloatRect localBounds = m_sprite.getLocalBounds();
	m_sprite.setOrigin(localBounds.width / 2.0f, localBounds.height / 2.0f);
}

void Sprite::setScale(sf::Vector2f scale)
{
	m_sprite.setScale(scale);
}

void Sprite::render(sf::RenderWindow & window)
{
	if (m_isActive)
	{
		window.draw(m_sprite);
	}
}

void Sprite::render(sf::RenderWindow & window, const Map & map)
{
	if (m_isActive)
	{
		const sf::Vector2i tileTransform = map.getTileScreenPos(sf::Vector2i(m_position.x, m_position.y));

		m_sprite.setPosition({
		static_cast<float>(tileTransform.x + DRAW_OFFSET_X * map.getDrawScale()),
		static_cast<float>(tileTransform.y + DRAW_OFFSET_Y * map.getDrawScale()) });
		window.draw(m_sprite);
	}
}

void Sprite::setTexture(std::unique_ptr<Texture>& texture, int frameID)
{
	assert(!m_texture);
	m_texture = texture.get();
	m_sprite.setTexture(m_texture->m_texture);
	setFrameID(frameID);
	setOriginAtCenter();
}

void Sprite::activate()
{
	m_isActive = true;
}

void Sprite::deactivate()
{
	m_isActive = false;
}