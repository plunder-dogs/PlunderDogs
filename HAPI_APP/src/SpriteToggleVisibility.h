#pragma once

#include <memory>
#include <HAPISprites_lib.h>
#include <HAPISprites_UI.h>

enum FactionName;
class Map;

struct SpriteToggleVisibility
{
	SpriteToggleVisibility(SpriteToggleVisibility& orig);
	//General sprite
	SpriteToggleVisibility(std::shared_ptr<SpriteSheet>& spriteSheet, float scaleX, float scaleY, bool active = true);
	//Sprite belonging to a faction
	SpriteToggleVisibility(FactionName factionName, float scaleX, float scaleY, bool active = true);

	void render(const Map& map, std::pair<int, int> position) const;
	void render(const Map& map) const;

	std::unique_ptr<Sprite> m_sprite;
	bool m_active;
	std::pair<int, int> m_position;
};