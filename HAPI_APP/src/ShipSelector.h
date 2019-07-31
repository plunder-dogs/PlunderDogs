#pragma once

#include "Sprite.h"
#include "Global.h"
#include <SFML/Graphics.hpp>
#include "Utilities/NonCopyable.h"
#include <vector>

struct SelectedShip
{
	SelectedShip(ShipOnTile shipOnTile, sf::Vector2i shipPosition);

	ShipOnTile m_shipOnTile;
	Sprite m_sprite;
};

class Map;
class Ship;
class ShipSelector : private NonCopyable
{
public:
	ShipSelector();

	const std::vector<SelectedShip>& getSelectedShips() const;
	//Get selected ship that has been removed
	ShipOnTile removeSelectedShip();

	void update(const std::vector<Ship>& currentFactionShips, sf::Vector2i mousePosition, const Map& map);
	void render(sf::RenderWindow& window, const Map& map, bool leftClickHeld);
	
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