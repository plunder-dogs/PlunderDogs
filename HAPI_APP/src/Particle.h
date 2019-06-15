#pragma once

#include <Sprite.h>
#include "Timer.h"

class Map;
enum eDirection;
struct Particle
{
	Particle(float lifespan, std::unique_ptr<Texture>& texture, float scale);
	void setPosition(sf::Vector2i position);
	void update(float deltaTime, const Map& map);
	void render(sf::RenderWindow& window, const Map& map);
	void orient(eDirection direction);

	sf::Vector2i m_position;
	Timer m_lifeSpan;
	Sprite m_sprite;
	int m_frameNum = 0;
	bool m_isEmitting;
	const float m_scale;
};