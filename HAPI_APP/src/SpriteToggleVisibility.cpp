#include "SpriteToggleVisibility.h"
#include "Global.h"
#include "Textures.h"
#include "Map.h"

constexpr float DRAW_ENTITY_OFFSET_X{ 16 };
constexpr float DRAW_ENTITY_OFFSET_Y{ 32 };

SpriteToggleVisibility::SpriteToggleVisibility(std::shared_ptr<SpriteSheet>& spriteSheet, float scaleX, float scaleY, bool active)
	: m_sprite(std::make_unique<Sprite>(spriteSheet)),
	m_active(active),
	m_position(0, 0)
{
	m_sprite->GetTransformComp().SetScaling({ scaleX, scaleY });
	m_sprite->GetTransformComp().SetOriginToCentreOfFrame();
}

SpriteToggleVisibility::SpriteToggleVisibility(FactionName factionName, float scaleX, float scaleY, bool active)
	: m_sprite(),
	m_active(active),
	m_position(0, 0)
{
	switch (factionName)
	{
	case FactionName::eYellow:
		m_sprite = HAPI_Sprites.MakeSprite(Textures::m_yellowSpawnHex);
		break;
	case FactionName::eBlue:
		m_sprite = HAPI_Sprites.MakeSprite(Textures::m_blueSpawnHex);
		break;
	case FactionName::eGreen:
		m_sprite = HAPI_Sprites.MakeSprite(Textures::m_greenSpawnHex);
		break;
	case FactionName::eRed:
		m_sprite = HAPI_Sprites.MakeSprite(Textures::m_redSpawnHex);
		break;
	}

	m_sprite->GetTransformComp().SetScaling({ scaleX, scaleY });
	m_sprite->GetTransformComp().SetOriginToCentreOfFrame();
}

SpriteToggleVisibility::SpriteToggleVisibility(SpriteToggleVisibility & orig)
	: m_active(orig.m_active),
	m_position(orig.m_position)
{
	m_sprite.swap(orig.m_sprite);
}

void SpriteToggleVisibility::render(const Map & map, std::pair<int, int> position) const
{
	if (m_active)
	{
		auto screenPosition = map.getTileScreenPos(position);
		m_sprite->GetTransformComp().SetPosition({
		static_cast<float>(screenPosition.first + DRAW_ENTITY_OFFSET_X * map.getDrawScale()) ,
		static_cast<float>(screenPosition.second + DRAW_ENTITY_OFFSET_Y * map.getDrawScale()) });
		m_sprite->Render(SCREEN_SURFACE);
	}
}

void SpriteToggleVisibility::render(const Map & map) const
{
	if (m_active)
	{
		std::pair<int, int> tileTransform = map.getTileScreenPos(m_position);
		m_sprite->GetTransformComp().SetPosition({
		static_cast<float>(tileTransform.first + DRAW_ENTITY_OFFSET_X * map.getDrawScale()),
		static_cast<float>(tileTransform.second + DRAW_ENTITY_OFFSET_Y * map.getDrawScale()) });

		m_sprite->Render(SCREEN_SURFACE);
	}
}