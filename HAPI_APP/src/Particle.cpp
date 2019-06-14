#include "Particle.h"
#include "Global.h"
#include "Map.h"

Particle::Particle(float lifespan, std::unique_ptr<Texture>& texture, float scale)
	: m_position(),
	m_lifeSpan(lifespan),
	m_sprite(texture),
	m_frameNum(0),
	m_isEmitting(false),
	m_scale(scale)
{
	//m_particle->SetFrameNumber(m_frameNum);
}

void Particle::setPosition(sf::Vector2i position)
{
	m_position = position;
}

void Particle::update(float deltaTime, const Map& map)
{
	if (m_isEmitting)
	{
		const sf::Vector2i tileTransform = map.getTileScreenPos(m_position);
		m_sprite.setPosition(sf::Vector2i(
			tileTransform.x + DRAW_OFFSET_X * map.getDrawScale(),
			tileTransform.y + DRAW_OFFSET_Y * map.getDrawScale()));

		m_lifeSpan.update(deltaTime);

		if (m_lifeSpan.isExpired())
		{
			m_sprite.setFrameID(m_frameNum);
			m_lifeSpan.reset();
			++m_frameNum;
		}

		if (m_frameNum >= m_sprite.getCurrentFrameID())
		{
			m_isEmitting = false;
			m_frameNum = 0;
		}
	}
}

void Particle::render(sf::RenderWindow& window)
{
	if (m_isEmitting)
	{
		//m_sprite->GetTransformComp().SetOriginToCentreOfFrame();
		m_sprite.setScale(sf::Vector2f(m_scale, m_scale));
		m_sprite.render(window);
	}
}

void Particle::orient(eDirection entityDir)
{
	eDirection direction = eNorth;
	switch (entityDir)
	{
	case eNorth:
		direction = eSouth;
		break;
	case eNorthEast:
		direction = eSouthWest;
		break;
	case eSouthEast:
		direction = eNorthWest;
		break;
	case eSouth:
		direction = eNorth;
		break;
	case eSouthWest:
		direction = eNorthEast;
		break;
	case eNorthWest:
		direction = eSouthEast;
		break;
	}

#ifdef SFML_REFACTOR - Ryan Swann
	m_sprite->GetTransformComp().SetRotation(DEGREES_TO_RADIANS(static_cast<int>(direction) * 60 % 360));
#endif 
}