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
	eLevelOneSelect,
	eLevelTwoSelect,
	eLevelThreeSelect,
	eLevelFourSelect,
	eLevelFiveSelect,
	ePlaySinglePlayer,
	ePlayerMultiplayer,
	eBack,
	eDone,
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
	UIComponentTextBox(const std::string& message, const sf::Font& font, sf::Vector2i position, eUIComponentName name);

	sf::Text text;
};

struct UIComponentButton : public UIComponentBase
{
	UIComponentButton(const Texture& texture, sf::Vector2i position, eUIComponentName name, bool changeOnIntersect = false);

	Sprite sprite;
	bool currentlyIntersected;
	bool changeOnIntersect;
};

class UIComponentIntersectionDetails
{
	friend class UILayer;
public:
	UIComponentIntersectionDetails();
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