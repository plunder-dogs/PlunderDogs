#include "Textures.h"
#include "Global.h"
#include "Utilities/XMLParser.h"
#include <assert.h>
#include <iostream>
#include <fstream>
#include <sstream>

const std::string DATA_DIRECTORY = "Data/";
const std::string SHIP_DATA_DIRECTORY = "Data/Ships/";

bool Textures::loadAllTextures()
{
	assert(!m_allTexturesLoaded);
	std::ifstream file("textures.txt");
	if (!file.is_open())
	{
		std::cerr << "Failed to open: " << "textures.txt" << "\n";
		return false;
	}

	std::string line;
	while (std::getline(file, line))
	{
		std::stringstream keyStream(line);

		std::string fileName;
		std::string directory;
		keyStream >> fileName >> directory;

		if (!loadTexture(fileName, directory))
		{
			std::cerr << "Failed to load: " << directory + fileName << "\n";
			return false;
		}
	}

	m_allTexturesLoaded = true;
	return true;
}

const Texture & Textures::getTexture(const std::string & textureName) const
{
	auto texture = m_textures.find(textureName);
	assert(texture != m_textures.cend());

	return *texture->second;
}

bool Textures::loadTexture(const std::string & fileName, const std::string & directory)
{
	std::string imagePath;
	std::vector<FrameDetails> frames;
	if (!XMLParser::loadTextureDetails(directory + fileName, imagePath, frames))
	{
		return false;
	}

	std::unique_ptr<Texture> texture = Texture::load(directory + imagePath, std::move(frames));
	if (texture)
	{
		auto iter = m_textures.find(fileName);
		assert(iter == m_textures.cend());
		m_textures.emplace(fileName, std::move(texture));

		return true;
	}

	return false;
}