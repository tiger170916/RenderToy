#include "TextureManager2.h"

TextureManager2::TextureManager2() 
{}

TextureManager2::~TextureManager2()
{}

Texture2* TextureManager2::GetOrCreateTexture(std::string name, std::filesystem::path filePath, uint32_t fileOffset, uint32_t dataSize, uint32_t width, uint32_t height, uint32_t numChannels)
{
	if (name.empty())
	{
		return nullptr;
	}

	if (!m_textures.contains(name))
	{
		m_textures[name] = std::make_unique<Texture2>(name, filePath, fileOffset, dataSize, width, height, numChannels);
	}

	return m_textures[name].get();
}

Texture2* TextureManager2::GetTexture(std::string name)
{
	if (m_textures.contains(name))
	{
		return m_textures[name].get();
	}

	return nullptr;
}

bool TextureManager2::StreamInBinary(GraphicsContext* graphicsContext, CommandBuilder* cmdBuilder)
{
	for (auto& texture : m_textures)
	{
		texture.second->StreamIn(graphicsContext, cmdBuilder);
	}

	return true;
}

void TextureManager2::SetStreamedInDone()
{
	for (auto& texture : m_textures)
	{
		texture.second->SetStreamedIn(true);
	}
}

bool TextureManager2::StreamOut() 
{
	return false;
}

bool TextureManager2::CleanUpAfterStreamIn() 
{
	return false;
}