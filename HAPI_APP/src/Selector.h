#pragma once

#include "ShipOnTile.h"
#include "Sprite.h"
#include <SFML/Graphics.hpp>
#include <array>
#include <vector>

constexpr size_t MAX_SHIPS_SELECT = 6;

struct SelectedShip
{
	SelectedShip();

	void add(ShipOnTile shipOnTile, sf::Vector2i position);
	void clear();

	ShipOnTile m_shipOnTile;
	Sprite m_sprite;
};

class Map;
class Ship;
class Selector
{
public:
	Selector();
	Selector(const Selector&) = delete;
	Selector& operator=(const Selector&) = delete;
	Selector(Selector&&) = delete;
	Selector&& operator=(Selector&&) = delete;

	bool isShipsSelected() const;
	const std::array<SelectedShip, MAX_SHIPS_SELECT>& getSelectedShips() const;

	void setPosition(sf::Vector2i position);
	void update(const std::vector<Ship>& currentFactionShips, sf::Vector2i mousePosition, const Map& map);
	void renderShipHighlight(sf::RenderWindow& window, const Map& map);
	void renderSelector(sf::RenderWindow& window);
	void reset();
	void resetShape();

private:
	std::array<SelectedShip, MAX_SHIPS_SELECT> m_selectedShips;
	sf::RectangleShape m_shape;
	sf::FloatRect m_AABB;
	
	void addToSelector(ShipOnTile shipToAdd, sf::Vector2i position);
	void removeFromSelector(ShipOnTile shipToRemove);
};