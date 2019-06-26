#pragma once

#include <Sprite.h>
#include "Timer.h"

class Map;
enum eDirection;
struct Particle
{
	Particle(float lifespan, const Texture& texture, float scale);

	void setPosition(sf::Vector2i position);
	void update(float deltaTime, const Map& map);
	void render(sf::RenderWindow& window, const Map& map);
	void orient(eDirection direction);

	Timer m_lifeSpan;
	Sprite m_sprite;
	const float m_scale;
};