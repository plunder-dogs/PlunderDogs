#pragma once

#include <memory>
#include <SFML/Graphics.hpp>

enum FactionName;
class Map;

struct SpriteToggleVisibility
{
	SpriteToggleVisibility(SpriteToggleVisibility& orig);
	//General sprite
	SpriteToggleVisibility(std::unique_ptr<sf::Texture>& spriteSheet, float scaleX, float scaleY, bool active = true);
	//Sprite belonging to a faction
	SpriteToggleVisibility(FactionName factionName, float scaleX, float scaleY, bool active = true);

	void render(const Map& map, sf::Vector2i position) const;
	void render(const Map& map) const;

	sf::Sprite m_sprite;
	bool m_active;
	sf::Vector2i m_position;
};