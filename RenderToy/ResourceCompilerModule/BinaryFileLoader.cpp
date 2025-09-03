#include "BinaryFileLoader.h"
#include <fstream>

bool BinaryFileLoader::LoadBinary(std::filesystem::path filePath)
{
	if (!std::filesystem::exists(filePath))
	{
		return false;
	}

	std::fstream file(filePath, std::ios::in | std::ios::binary);

	std::byte header[1024];
	file.read((char*)header, 1024);
	TileHeader* pTileHeader = (TileHeader*)header;

	if (pTileHeader->MagicNum != 0x12345678)
	{
		// Wrong file format
		return false;
	}

	std::vector<StaticMeshHeader> staticMeshHeaders(pTileHeader->NumStaticMeshes);
	std::vector<StaticMeshInstanceHeader> staticMeshInstanceHeaders(pTileHeader->NumStaticMeshInstances);
	std::vector<LightExtensionHeader> lightExtensionHeaders(pTileHeader->NumLightExtensions);

	char* stringBuf = (char*)malloc(pTileHeader->StringTotalLength * sizeof(char));

	file.read((char*)staticMeshHeaders.data(), pTileHeader->NumStaticMeshes * sizeof(StaticMeshHeader));
	file.read((char*)staticMeshInstanceHeaders.data(), pTileHeader->NumStaticMeshInstances * sizeof(StaticMeshInstanceHeader));
	file.read((char*)lightExtensionHeaders.data(), pTileHeader->NumLightExtensions * sizeof(LightExtensionHeader));
	file.read(stringBuf, pTileHeader->StringTotalLength);

	file.close();
	free(stringBuf);

	return true;
}