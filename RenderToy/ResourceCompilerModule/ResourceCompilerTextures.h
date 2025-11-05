#pragma once

#include <string>
#include <vector>

namespace ResourceCompilerModule
{
	class Texture
	{
	public:
		virtual std::string GetTextureName() = 0;

		virtual uint32_t GetTextureDataOffset() = 0;

		virtual uint32_t GetTextureDataSize() = 0;

		virtual uint32_t GetHeight() = 0;

		virtual uint32_t GetWidth() = 0;

		virtual uint32_t GetNumChannels() = 0;
	};

	class TexturesData
	{
	public:
		virtual Texture* GetTextureByName(std::string name) = 0;

		virtual void GetTextures(std::vector<Texture*>& outTextures) = 0;
	};
}