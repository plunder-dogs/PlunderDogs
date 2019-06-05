#pragma once

#include <SFML/Graphics.hpp>

//std::string tileSheetName = tileSheetElement->Attribute("name");
//sf::Vector2i tileSetSize;
//int spacing = 0, margin = 0, tileSize = 0, firstGID = 0;
//tileSheetElement->FirstChildElement()->Attribute("width", &tileSetSize.x);
//tileSheetElement->FirstChildElement()->Attribute("height", &tileSetSize.y);
//tileSheetElement->Attribute("tilewidth", &tileSize);
//tileSheetElement->Attribute("spacing", &spacing);
//tileSheetElement->Attribute("firstgid", &firstGID);
//tileSheetElement->Attribute("margin", &margin);
//const int columns = tileSetSize.x / (tileSize + spacing);
//const int rows = tileSetSize.y / (tileSize + spacing);

struct Texture
{
	Texture(sf::Vector2i tileSize, sf::Vector2i size, int rows, int columns)
		: m_tileSize(tileSize),
		m_size(size),
		m_rows(rows),
		m_columns(columns)
	{
		m_texture.loadFromFile()
	}
	
	sf::Texture m_texture;
	const sf::Vector2i m_tileSize;
	const sf::Vector2i m_size;
	const int m_rows;
	const int m_columns;
};