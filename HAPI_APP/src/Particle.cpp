#include "Particle.h"
#include "Global.h"
#include "Map.h"

Particle::Particle(float lifespan, const Texture& texture, float scale)
	: m_lifeSpan(lifespan),
	m_sprite(texture, false),
	m_scale(scale)
{
	m_sprite.setFrameID(0);
}

void Particle::setPosition(sf::Vector2i position)
{
	m_sprite.setPosition(position);
}

void Particle::update(float deltaTime, const Map& map)
{
	if (m_sprite.isActive())
	{
		m_lifeSpan.update(deltaTime);

		if (m_lifeSpan.isExpired())
		{
			m_sprite.incrementFrameID();
			m_lifeSpan.reset();
		}

		if (m_sprite.isAnimationCompleted())
		{
			m_sprite.deactivate();
			m_sprite.setFrameID(0);
		}
	}
}

void Particle::render(sf::RenderWindow& window, const Map& map)
{
	m_sprite.setScale(sf::Vector2f(m_scale, m_scale));
	m_sprite.render(window, map);	
}

void Particle::orient(eDirection entityDir)
{
	eDirection direction = eDirection::eNorth;
	switch (entityDir)
	{
	case eDirection::eNorth:
		direction = eDirection::eSouth;
		break;
	case eDirection::eNorthEast:
		direction = eDirection::eSouthWest;
		break;
	case eDirection::eSouthEast:
		direction = eDirection::eNorthWest;
		break;
	case eDirection::eSouth:
		direction = eDirection::eNorth;
		break;
	case eDirection::eSouthWest:
		direction = eDirection::eNorthEast;
		break;
	case eDirection::eNorthWest:
		direction = eDirection::eSouthEast;
		break;
	}

#ifdef SFML_REFACTOR - Ryan Swann
	m_sprite->GetTransformComp().SetRotation(DEGREES_TO_RADIANS(static_cast<int>(direction) * 60 % 360));
#endif 
}