#pragma once
#include "ResourceCompilerTextures.h"
#include <map>
#include <memory>

using namespace ResourceCompilerModule;

class TextureImpl : public Texture
{
private:
	std::string m_name;

	uint32_t m_dataOffset;
	uint32_t m_dataSize;
	uint32_t m_height;
	uint32_t m_width;
	uint32_t m_numChannels;

public:
	inline virtual std::string GetTextureName() override { return m_name; }

	inline virtual uint32_t GetTextureDataOffset() override { return m_dataOffset; }

	inline virtual uint32_t GetTextureDataSize() override { return m_dataSize; }

	inline virtual uint32_t GetHeight() override { return m_height; }

	inline virtual uint32_t GetWidth() override { return m_width; }

	inline virtual uint32_t GetNumChannels() override { return m_numChannels; }

	TextureImpl(
		const std::string& name,
		const uint32_t& dataOffset,
		const uint32_t& dataSize, 
		const uint32_t& width,
		const uint32_t& height, 
		const uint32_t& numChannels);
};

class TexturesDataImpl : public TexturesData
{
	std::map<std::string, std::shared_ptr<TextureImpl>> m_textures;
public:
	virtual Texture* GetTextureByName(std::string name) override;

	virtual void GetTextures(std::vector<Texture*>& outTextures) override;

	void AddTexture(std::shared_ptr<TextureImpl> texture);
};