#include "ShipSelector.h"
#include "Resources.h"
#include "Battle.h"
#include <assert.h>
#include <math.h>

constexpr int MIN_SHAPE_WIDTH_SIZE = 25;
constexpr int MIN_SHAPE_HEIGHT_SIZE = 25;

SelectedShip::SelectedShip(ShipOnTile shipOnTile, sf::Vector2i shipPosition)
	: m_shipOnTile(shipOnTile),
	m_sprite(Textures::getInstance().getTexture(SELECTED_HEX))
{
	m_sprite.setPosition(shipPosition);
}

ShipSelector::ShipSelector()
	: m_selectedShips(),
	m_shape(),
	m_AABB()
{
	m_selectedShips.reserve(MAX_SHIPS_PER_FACTION);

	m_shape.setFillColor(sf::Color::Transparent);
	m_shape.setOutlineColor(sf::Color::Green);
	m_shape.setOutlineThickness(1.5f);
}

const std::vector<SelectedShip>& ShipSelector::getSelectedShips() const
{
	return m_selectedShips;
}

ShipOnTile ShipSelector::removeSelectedShip()
{
	assert(!m_selectedShips.empty());

	ShipOnTile removedSelectedShip = m_selectedShips.back().m_shipOnTile;
	m_selectedShips.pop_back();

	return removedSelectedShip;
}

void ShipSelector::setPosition(sf::Vector2i position)
{
	m_shape.setPosition(sf::Vector2f(position.x, position.y));
	m_AABB.left = position.x;
	m_AABB.top = position.y;
}

void ShipSelector::update(const Battle& battle, sf::Vector2i mousePosition, const Map& map)
{
	//Change Size
	mousePosition -= MOUSE_POSITION_OFFSET;
	sf::Vector2f selectorSize(static_cast<float>(mousePosition.x) - m_shape.getPosition().x,
		static_cast<float>(mousePosition.y) - m_shape.getPosition().y);

	m_shape.setSize(selectorSize);
	m_AABB.width = selectorSize.x;
	m_AABB.height = selectorSize.y;

	//Don't add AI or Remote Player ships
	if (battle.getCurrentFaction().m_controllerType == eFactionControllerType::eAI ||
		battle.getCurrentFaction().m_controllerType == eFactionControllerType::eRemotePlayer)
	{
		return;
	}

	//Add/Remove ships
	if (std::abs(m_shape.getSize().x) >= MIN_SHAPE_WIDTH_SIZE &&
		std::abs(m_shape.getSize().y) >= MIN_SHAPE_HEIGHT_SIZE)
	{
		for (const auto& shipToSelect : battle.getCurrentFaction().getAllShips())
		{
			//Do not add these ships to selector
			if (shipToSelect.isDead() || shipToSelect.isDestinationSet() || shipToSelect.isWeaponFired())
			{
				continue;
			}

			if (m_AABB.intersects(shipToSelect.getAABB(map)))
			{
				addToSelector({ shipToSelect.getFactionName(), shipToSelect.getID() }, shipToSelect.getCurrentPosition());
			}
			else
			{
				removeFromSelector({ shipToSelect.getFactionName(), shipToSelect.getID() });
			}
		}
	}
}

void ShipSelector::render(sf::RenderWindow & window, const Map & map, bool leftClickHeld)
{
	for (auto& selectedShip : m_selectedShips)
	{
		selectedShip.m_sprite.render(window, map);
	}

	if (leftClickHeld)
	{
		window.draw(m_shape);
	}
}

void ShipSelector::reset()
{
	m_shape.setSize(sf::Vector2f(0, 0));
	m_AABB.width = 0;
	m_AABB.height = 0;

	m_selectedShips.clear();
}

void ShipSelector::resetShape()
{
	m_shape.setSize(sf::Vector2f(0, 0));
	m_AABB.width = 0;
	m_AABB.height = 0;
}

void ShipSelector::resetShape(sf::Vector2i position)
{
	position -= MOUSE_POSITION_OFFSET;
	setPosition(position);
	m_shape.setSize(sf::Vector2f(0, 0));
	m_AABB.width = 0;
	m_AABB.height = 0;
}

void ShipSelector::addToSelector(ShipOnTile shipToAdd, sf::Vector2i shipPosition)
{
	auto cIter = std::find_if(m_selectedShips.cbegin(), m_selectedShips.cend(), [shipToAdd](const auto& selectedShip)
		{ return selectedShip.m_shipOnTile.shipID == shipToAdd.shipID; });
	
	if (cIter == m_selectedShips.cend())
	{
		m_selectedShips.emplace_back(shipToAdd, shipPosition);
		assert(m_selectedShips.size() <= MAX_SHIPS_PER_FACTION);
	}
}

void ShipSelector::removeFromSelector(ShipOnTile shipToRemove)
{
	auto cIter = std::find_if(m_selectedShips.cbegin(), m_selectedShips.cend(), [shipToRemove](const auto& selectedShip)
		{ return selectedShip.m_shipOnTile.shipID == shipToRemove.shipID; });
	
	if (cIter != m_selectedShips.cend())
	{
		m_selectedShips.erase(cIter);
	}
}