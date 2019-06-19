#pragma once

#include "ShipOnTile.h"
#include "Sprite.h"
#include <SFML/Graphics.hpp>
#include <array>
#include <vector>

constexpr size_t MAX_SHIPS_SELECT = 6;

class Map;
class Ship;
class Selector
{
	struct SelectedShip
	{
		SelectedShip();

		ShipOnTile shipOnTile;
		Sprite sprite;
	};
	
public:
	Selector();
	Selector(const Selector&) = delete;
	Selector& operator=(const Selector&) = delete;
	Selector(Selector&&) = delete;
	Selector&& operator=(Selector&&) = delete;

	void setPosition(sf::Vector2i position);
	void update(const std::vector<Ship>& currentFactionShips, sf::Vector2i mousePosition, const Map& map);
	void render(sf::RenderWindow& window);
	void reset();

private:
	std::array<SelectedShip, MAX_SHIPS_SELECT> m_selectedShips;
	sf::RectangleShape m_shape;
	sf::FloatRect m_AABB;
	
	void addToSelector(ShipOnTile shipToAdd, sf::Vector2i position);
	void removeFromSelector(ShipOnTile shipToRemove);
};