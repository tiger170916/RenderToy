#include "TextureManager.h"

TextureManager::TextureManager()
{
	m_criticalSection = std::unique_ptr<CriticalSection>(new CriticalSection());
}

std::shared_ptr<Texture> TextureManager::LoadTexture(std::filesystem::path path)
{
	if (!std::filesystem::exists(path))
	{
		return nullptr;
	}

	m_criticalSection->EnterCriticalSection();

	if (m_textures.contains(path))
	{
		return m_textures[path];
	}

	m_textures[path] = std::shared_ptr<Texture>(new Texture(path));

	m_criticalSection->ExitCriticalSection();

	return m_textures[path];
}

TextureManager::~TextureManager()
{

}