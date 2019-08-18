#pragma once

#include "Utilities/NonCopyable.h"
#include <SFML/Graphics.hpp>
#include <array>
#include <vector>
#include <memory>
#include "Global.h"
#include "Sprite.h"
#include <string>

enum class eUIComponentName
{
	eYellowFactionSelect = 0,
	eBlueFactionSelect,
	eGreenFactionSelect,
	eRedFactionSelect,
	ePlaySinglePlayer,
	ePlayerMultiplayer,
	eBack,
	eQuit
};

enum class eUIComponentType
{
	eTextBox = 0,
	eButton
};

struct UIComponentBase
{
	const eUIComponentName name;
	sf::IntRect AABB;

protected:
	UIComponentBase(eUIComponentName name)
		: name(name),
		AABB()
	{}
};

struct UIComponentTextBox : public UIComponentBase
{
	UIComponentTextBox(const std::string& message, const sf::Font& font, sf::Vector2i position, eUIComponentName name)
		: UIComponentBase(name),
		text(message, font)
	{
		text.setPosition(sf::Vector2f(position.x, position.y));

		AABB.left = position.x - 5;
		AABB.top = position.y - 5;
		AABB.width = static_cast<int>(text.getLocalBounds().width) + 5;
		AABB.height = static_cast<int>(text.getLocalBounds().height) + 5;
	}

	sf::Text text;
};

struct UIComponentButton : public UIComponentBase
{
	UIComponentButton(const Texture& texture, sf::Vector2i position, eUIComponentName name)
		: UIComponentBase(name),
		sprite(texture, position, true, false)
	{
		AABB.left = position.x;
		AABB.top = position.y;
		AABB.width = static_cast<int>(sprite.getSize().x);
		AABB.height = static_cast<int>(sprite.getSize().y);
	}

	Sprite sprite;
};

class UIComponentIntersectionDetails
{
	friend class UILayer;
public:
	UIComponentIntersectionDetails(bool intersected);
	UIComponentIntersectionDetails(bool intersected, int frameID);

	bool isIntersected() const;
	int getFrameID() const;
	eUIComponentType getComponentType();
	eUIComponentName getComponentName();

private:
	bool m_intersected;
	eUIComponentType m_type;
	eUIComponentName m_name;
	int m_frameID;
};