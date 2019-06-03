#include "SpriteToggleVisibility.h"
#include "Global.h"
#include "Textures.h"
#include "Map.h"

constexpr float DRAW_ENTITY_OFFSET_X{ 16 };
constexpr float DRAW_ENTITY_OFFSET_Y{ 32 };

SpriteToggleVisibility::SpriteToggleVisibility(std::unique_ptr<sf::Texture>& texture, float scaleX, float scaleY, bool active)
	: m_sprite(*texture),
	m_active(active),
	m_position(0, 0)
{
	m_sprite.setScale(scaleX, scaleY);
	//m_sprite->GetTransformComp().SetOriginToCentreOfFrame();
}

SpriteToggleVisibility::SpriteToggleVisibility(FactionName factionName, float scaleX, float scaleY, bool active)
	: m_sprite(),
	m_active(active),
	m_position(0, 0)
{
	switch (factionName)
	{
	case FactionName::eYellow:
		m_sprite.setTexture(*Textures::m_yellowSpawnHex);
		break;
	case FactionName::eBlue:
		m_sprite.setTexture(*Textures::m_blueSpawnHex);
		break;
	case FactionName::eGreen:
		m_sprite.setTexture(*Textures::m_greenSpawnHex);
		break;
	case FactionName::eRed:
		m_sprite.setTexture(*Textures::m_redSpawnHex);
		break;
	}
	m_sprite.setScale(scaleX, scaleY);
	//m_sprite->GetTransformComp().SetOriginToCentreOfFrame();
}

SpriteToggleVisibility::SpriteToggleVisibility(SpriteToggleVisibility & orig)
	: m_sprite(orig.m_sprite),
	m_active(orig.m_active),
	m_position(orig.m_position)
{}

void SpriteToggleVisibility::render(sf::RenderWindow& window, const Map & map, sf::Vector2i position)
{
	if (m_active)
	{
		auto screenPosition = map.getTileScreenPos(position);
		m_sprite.setPosition({
		static_cast<float>(screenPosition.x + DRAW_ENTITY_OFFSET_X * map.getDrawScale()) ,
		static_cast<float>(screenPosition.y + DRAW_ENTITY_OFFSET_Y * map.getDrawScale()) });
		
		window.draw(m_sprite);
	}
}

void SpriteToggleVisibility::render(sf::RenderWindow& window, const Map & map)
{
	if (m_active)
	{
		sf::Vector2i tileTransform = map.getTileScreenPos(m_position);
		m_sprite.setPosition({
		static_cast<float>(tileTransform.x + DRAW_ENTITY_OFFSET_X * map.getDrawScale()),
		static_cast<float>(tileTransform.y + DRAW_ENTITY_OFFSET_Y * map.getDrawScale()) });

		window.draw(m_sprite);
	}
}