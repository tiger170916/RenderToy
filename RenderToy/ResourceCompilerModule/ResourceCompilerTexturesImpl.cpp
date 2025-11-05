#include "ResourceCompilerTexturesImpl.h"

TextureImpl::TextureImpl(
	const std::string& name,
	const uint32_t& dataOffset,
	const uint32_t& dataSize,
	const uint32_t& width,
	const uint32_t& height,
	const uint32_t& numChannels) 
	: m_name(name), m_dataOffset(dataOffset), m_dataSize(dataSize), m_width(width), m_height(height), m_numChannels(numChannels)
{
}

Texture* TexturesDataImpl::GetTextureByName(std::string name)
{
	if (!m_textures.contains(name))
	{
		return nullptr;
	}

	return m_textures[name].get();
}

void TexturesDataImpl::GetTextures(std::vector<Texture*>& outTextures)
{
	outTextures.clear();
	for (auto& pair : m_textures)
	{
		outTextures.push_back(pair.second.get());
	}
}

void TexturesDataImpl::AddTexture(std::shared_ptr<TextureImpl> texture)
{
	if (!texture)
	{
		return;
	}

	if (m_textures.contains(texture->GetTextureName()))
	{
		return;
	}

	m_textures[texture->GetTextureName()] = texture;
}