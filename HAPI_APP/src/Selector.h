#pragma once

#include "ShipOnTile.h"
#include "Sprite.h"
#include <SFML/Graphics.hpp>
#include <vector>

struct SelectedShip
{
	SelectedShip(ShipOnTile shipOnTile, sf::Vector2i shipPosition);

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

	const std::vector<SelectedShip>& getSelectedShips() const;
	ShipOnTile removeSelectedShip();

	void update(const std::vector<Ship>& currentFactionShips, sf::Vector2i mousePosition, const Map& map);
	void renderShipHighlight(sf::RenderWindow& window, const Map& map);
	void renderSelector(sf::RenderWindow& window);
	
	void reset();
	void resetShape();
	//Reset shape at position
	void resetShape(sf::Vector2i position);

private:
	std::vector<SelectedShip> m_selectedShips;
	sf::RectangleShape m_shape;
	sf::FloatRect m_AABB;
	
	void addToSelector(ShipOnTile shipToAdd, sf::Vector2i shipPosition);
	void removeFromSelector(ShipOnTile shipToRemove);
	void setPosition(sf::Vector2i position);
};