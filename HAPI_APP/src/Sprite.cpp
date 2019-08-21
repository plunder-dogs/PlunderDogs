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

Sprite::Sprite(const Texture & texture, sf::Vector2i startingPosition, bool active, bool originAtCentre)
	: m_texture(&texture),
	m_sprite(),
	m_position(startingPosition),
	m_currentFrameID(0),
	m_isActive(active)
{
	m_sprite.setPosition(sf::Vector2f(startingPosition.x, startingPosition.y));
	m_sprite.setTexture(m_texture->getTexture());
	setFrameID(m_currentFrameID);
	if (originAtCentre)
	{
		setOriginAtCenter();
	}
}

Sprite::Sprite(const Texture& texture, bool active, bool setOriginAtCentre)
	: m_texture(&texture),
	m_sprite(),
	m_position(),
	m_currentFrameID(0),
	m_isActive(active)
{
	m_sprite.setTexture(m_texture->getTexture());
	setFrameID(m_currentFrameID);
	if (setOriginAtCentre)
	{
		setOriginAtCenter();
	}
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
	return m_currentFrameID >= static_cast<int>(m_texture->getFrames().size()) - 1;
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
	++m_currentFrameID;
	if (m_currentFrameID >= m_texture->getFrames().size())
	{
		m_currentFrameID = 0;
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

void Sprite::render(sf::RenderWindow & window) const
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

void Sprite::setTexture(const Texture& texture, int frameID)
{
	m_texture = &texture;
	m_sprite.setTexture(m_texture->getTexture());
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