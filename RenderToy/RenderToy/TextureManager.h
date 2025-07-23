#pragma once
#include "Includes.h"
#include "Texture.h"
#include "CriticalSection.h"
#include "GuidComparator.h"

class TextureManager
{
private:
	std::unique_ptr<CriticalSection> m_criticalSection;

	std::map<std::filesystem::path, std::shared_ptr<Texture>> m_textures;

public:
	TextureManager();

	std::shared_ptr<Texture> LoadTexture(std::filesystem::path path);

	~TextureManager();
};