#include "Selector.h"
#include "Textures.h"
#include "Ship.h"
#include <assert.h>
#include <math.h>

constexpr int MIN_SHIP_SELECT_SIZE = 75;
SelectedShip::SelectedShip()
	: m_shipOnTile(),
	m_sprite()
{}

void SelectedShip::add(ShipOnTile shipOnTile, sf::Vector2i position)
{
	m_shipOnTile = shipOnTile;

	m_sprite.setPosition(position);
	m_sprite.activate();
}

void SelectedShip::clear()
{
	m_shipOnTile.clear();
	m_sprite.deactivate();
}

Selector::Selector()
	: m_selectedShips(),
	m_shape(),
	m_AABB()
{
	for (auto& selectedShip : m_selectedShips)
	{
		selectedShip.m_sprite.setTexture(Textures::getInstance().m_selectedHex);
	}

	m_shape.setFillColor(sf::Color::Transparent);
	m_shape.setOutlineColor(sf::Color::Green);
	m_shape.setOutlineThickness(1.5f);
}

bool Selector::isShipsSelected() const
{
	auto cIter = std::find_if(m_selectedShips.cbegin(), m_selectedShips.cend(), [](const auto& selectedShip) { return selectedShip.m_shipOnTile.isValid(); });
	return cIter != m_selectedShips.cend();
}

const std::array<SelectedShip, MAX_SHIPS_SELECT>& Selector::getSelectedShips() const
{
	return m_selectedShips;
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
	if (std::abs(m_shape.getSize().x) >= MIN_SHIP_SELECT_SIZE &&
		std::abs(m_shape.getSize().y) >= MIN_SHIP_SELECT_SIZE)
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

void Selector::renderShipHighlight(sf::RenderWindow & window, const Map& map)
{
	

	for (auto& selectedShip : m_selectedShips)
	{
		if (selectedShip.m_shipOnTile.isValid())
		{
			selectedShip.m_sprite.render(window, map);
		}
	}
}

void Selector::renderSelector(sf::RenderWindow & window)
{
	window.draw(m_shape);
}

void Selector::reset()
{
	m_shape.setSize(sf::Vector2f(0, 0));
	m_AABB.width = 0;
	m_AABB.height = 0;

	for (auto& selectedShip : m_selectedShips)
	{
		if (selectedShip.m_shipOnTile.isValid())
		{
			selectedShip.clear();
		}
	}
}

void Selector::resetShape()
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
		if (shipToAdd == selectedShip.m_shipOnTile)
		{
			break;
		}

		if (!selectedShip.m_shipOnTile.isValid())
		{
			selectedShip.add(shipToAdd, position);
			break;
		}
	}
}

void Selector::removeFromSelector(ShipOnTile shipToRemove)
{
	for (SelectedShip& selectedShip : m_selectedShips)
	{
		//Remove ship from selected
		if (shipToRemove == selectedShip.m_shipOnTile)
		{
			selectedShip.clear();
			break;
		}
	}
}