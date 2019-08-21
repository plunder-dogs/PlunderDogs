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
	eEndPhase,
	eBack,
	eDone,
	eAlert,
	eQuit,
	MainMenuBackground,
	eGameBackground,
	eFactionFlags,
	eYellowFactionText,
	eYellowFactionReadyText,
	eBlueFactionText,
	eBlueFactionReadyText,
	eGreenFactionText,
	eGreenFactionReadyText,
	eRedFactionText,
	eRedFactionReadyText,
	ePause,
	eReady,
	eShipStats,
	eShipStatHealth,
	eShipStatAttackRange,
	eShipStatMovementRange,
	eShipStatDamage
};

enum class eIsComponentVisible
{
	eFalse = 0,
	eTrue
};

enum class eIsComponentChangeOnIntersect
{
	eFalse = 0,
	eTrue
};

enum class eUIComponentType
{
	eTextBox = 0,
	eButton,
	eImage
};

struct UIComponentBase
{
	const eUIComponentName name;
	sf::IntRect AABB;
	bool visible;

protected:
	UIComponentBase(eUIComponentName name, eIsComponentVisible visible = eIsComponentVisible::eTrue)
		: name(name),
		AABB(),
		visible(static_cast<bool>(visible))
	{}
};

struct UIComponentImage : public UIComponentBase
{
	UIComponentImage(const Texture& texture, sf::Vector2i position, eUIComponentName name, eIsComponentVisible visible = eIsComponentVisible::eTrue);

	Sprite sprite;
};

struct UIComponentTextBox : public UIComponentBase
{
	UIComponentTextBox(const std::string& message, const sf::Font& font, sf::Vector2i position, eUIComponentName name, 
		eIsComponentVisible visible = eIsComponentVisible::eTrue, sf::Color textColor = sf::Color::White);

	sf::Text text;
};

struct UIComponentButton : public UIComponentImage
{
	UIComponentButton(const Texture& texture, sf::Vector2i position, eUIComponentName name, eIsComponentChangeOnIntersect changeOnIntersect = eIsComponentChangeOnIntersect::eFalse,
		eIsComponentVisible visible = eIsComponentVisible::eTrue);

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