#include "UIComponents.h"
#include <assert.h>

//UI Component Text Box
UIComponentTextBox::UIComponentTextBox(const std::string & message, const sf::Font & font, sf::Vector2i position, eUIComponentName name, eIsComponentVisible visible, sf::Color textColor)
	: UIComponentBase(name, visible),
	text(message, font)
{
	text.setPosition(sf::Vector2f(position.x, position.y));
	text.setColor(textColor);

	AABB.left = position.x - 5;
	AABB.top = position.y - 5;
	AABB.width = static_cast<int>(text.getLocalBounds().width) + 5;
	AABB.height = static_cast<int>(text.getLocalBounds().height) + 5;
}

//UI Component Button
UIComponentButton::UIComponentButton(const Texture & texture, sf::Vector2i position, eUIComponentName name, eIsComponentChangeOnIntersect changeOnIntersect, eIsComponentVisible visible)
	: UIComponentImage(texture, position, name, visible),
	currentlyIntersected(false),
	changeOnIntersect(static_cast<bool>(changeOnIntersect))
{}

//UI Component Intersection Details
UIComponentIntersectionDetails::UIComponentIntersectionDetails()
	: m_intersected(false),
	m_type(),
	m_name(),
	m_frameID(0)
{}

UIComponentIntersectionDetails::UIComponentIntersectionDetails(bool intersected, int frameID)
	: m_intersected(intersected),
	m_type(),
	m_name(),
	m_frameID(frameID)
{}

bool UIComponentIntersectionDetails::isIntersected() const
{
	return m_intersected;
}

int UIComponentIntersectionDetails::getFrameID() const
{
	assert(m_intersected);
	return m_frameID;
}

eUIComponentType UIComponentIntersectionDetails::getComponentType()
{
	assert(m_intersected);
	assert(m_type == eUIComponentType::eButton);
	return m_type;
}

eUIComponentName UIComponentIntersectionDetails::getComponentName()
{
	assert(m_intersected);
	return m_name;
}

UIComponentImage::UIComponentImage(const Texture & texture, sf::Vector2i position, eUIComponentName name, eIsComponentVisible visible)
	: UIComponentBase(name, visible),
	sprite(texture, position, true, false)
{
	AABB.left = position.x;
	AABB.top = position.y;
	AABB.width = static_cast<int>(sprite.getSize().x);
	AABB.height = static_cast<int>(sprite.getSize().y);
}