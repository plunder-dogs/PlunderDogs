#include "Selector.h"
#include "Textures.h"
#include "Ship.h"
#include <assert.h>

constexpr int MIN_SHIP_SELECT_SIZE = 75;

Selector::SelectedShip::SelectedShip()
	: shipOnTile(),
	sprite()
{}

Selector::Selector()
	: m_selectedShips(),
	m_shape(),
	m_AABB()
{
	for (auto& selectedShip : m_selectedShips)
	{
		selectedShip.sprite.setTexture(Textures::getInstance().m_selectedHex);
	}

	m_shape.setFillColor(sf::Color::Transparent);
	m_shape.setOutlineColor(sf::Color::Green);
	m_shape.setOutlineThickness(1.5f);
}

void Selector::setPosition(sf::Vector2i position)
{
	m_shape.setPosition(sf::Vector2f(position.x, position.y));
	m_AABB.left = position.x;
	m_AABB.top = position.y;
}

void Selector::update(const std::vector<Ship>& currentFactionShips, sf::Vector2i mousePosition, const Map& map)
{
	//Change Size
	sf::Vector2f selectorSize(static_cast<float>(mousePosition.x) - m_shape.getPosition().x,
		static_cast<float>(mousePosition.y) - m_shape.getPosition().y);
	m_shape.setSize(selectorSize);
	m_AABB.width = selectorSize.x;
	m_AABB.height = selectorSize.y;

	//Add/Remove ships
	if (m_shape.getSize().x >= MIN_SHIP_SELECT_SIZE &&
		m_shape.getSize().y >= MIN_SHIP_SELECT_SIZE)
	{
		for (auto& ship : currentFactionShips)
		{
			if (m_AABB.intersects(ship.getAABB(map)))
			{
				addToSelector(ShipOnTile(ship.getFactionName(), ship.getID()), ship.getCurrentPosition());
			}
			else
			{
				removeFromSelector(ShipOnTile(ship.getFactionName(), ship.getID()));
			}
		}
	}
}

void Selector::render(sf::RenderWindow & window, const Map& map)
{
	window.draw(m_shape);

	for (auto& selectedShip : m_selectedShips)
	{
		if (selectedShip.shipOnTile.isValid())
		{
			selectedShip.sprite.render(window, map);
		}
	}
}

void Selector::reset()
{
	m_shape.setSize(sf::Vector2f(0, 0));
	m_AABB.width = 0;
	m_AABB.height = 0;
}

void Selector::addToSelector(ShipOnTile shipToAdd, sf::Vector2i position)
{
	for (SelectedShip& selectedShip : m_selectedShips)
	{
		//Ship to add already exists
		if (shipToAdd == selectedShip.shipOnTile)
		{
			break;
		}

		if (!selectedShip.shipOnTile.isValid())
		{
			selectedShip.shipOnTile = shipToAdd;
			selectedShip.sprite.setPosition(position);
			selectedShip.sprite.activate();
			break;
		}
	}
}

void Selector::removeFromSelector(ShipOnTile shipToRemove)
{
	for (SelectedShip& selectedShip : m_selectedShips)
	{
		//Remove ship from selected
		if (shipToRemove == selectedShip.shipOnTile)
		{
			selectedShip.shipOnTile.clear();
			selectedShip.sprite.deactivate();
			break;
		}
	}
}