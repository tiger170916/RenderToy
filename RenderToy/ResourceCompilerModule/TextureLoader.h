#pragma once
#include <filesystem>
#include <fstream>
#include <wincodec.h>
#include <guiddef.h>
#include <wrl.h>
#include <map>
#include "GuidComparator.h"

using namespace Microsoft::WRL;

class TextureLoader
{
private:
	static std::map<GUID, uint32_t, GuidComparator> _pixelFormatLookup;
public:
	TextureLoader() = default;

	bool LoadTexture(
		std::filesystem::path, 
		std::ofstream* file, 
		uint32_t* textureOffset, 
		uint32_t& outWidth,
		uint32_t& outHeight, 
		uint32_t& outFileSize,
		uint32_t& outNumChannels);
};