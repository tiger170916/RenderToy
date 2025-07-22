#include "TextureManager.h"

TextureManager::TextureManager()
{
	m_criticalSection = std::unique_ptr<CriticalSection>(new CriticalSection());
}

Texture* TextureManager::LoadTexture(std::filesystem::path path)
{
	if (!std::filesystem::exists(path))
	{
		return nullptr;
	}

	m_criticalSection->EnterCriticalSection();

	if (m_textures.contains(path))
	{
		return m_textures[path].get();
	}

	Texture* newTexture = new Texture(path);
	m_textures[path] = std::unique_ptr<Texture>(newTexture);

	m_criticalSection->ExitCriticalSection();

	return newTexture;
}

TextureManager::~TextureManager()
{

}