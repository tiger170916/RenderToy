#include "BinaryFileLoader.h"

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
	std::vector<MeshPartHeader> meshPartHeaders(pTileHeader->NumMeshParts);

	file.read((char*)staticMeshHeaders.data(), pTileHeader->NumStaticMeshes * sizeof(StaticMeshHeader));
	file.read((char*)staticMeshInstanceHeaders.data(), pTileHeader->NumStaticMeshInstances * sizeof(StaticMeshInstanceHeader));
	file.read((char*)lightExtensionHeaders.data(), pTileHeader->NumLightExtensions * sizeof(LightExtensionHeader));
	file.read((char*)meshPartHeaders.data(), pTileHeader->NumMeshParts * sizeof(MeshPartHeader));


	file.close();

	return true;
}

bool BinaryFileLoader::ReadHeader(std::fstream* file, ResourceCompilerModule::TileHeader& header)
{
	if (!file)
	{
		return false;
	}
	 
	std::byte headerBytes[1024];
	file->read((char*)headerBytes, 1024);
	TileHeader* pTileHeader = (TileHeader*)headerBytes;

	if (pTileHeader->MagicNum != 0x12345678)
	{
		// Wrong file format
		return false;
	}

	memcpy(&header, headerBytes, sizeof(TileHeader));

	return true;
}

bool BinaryFileLoader::GetStaticMeshes(
	std::fstream* file,
	const ResourceCompilerModule::TileHeader& header,
	ResourceCompilerModule::StaticMeshHeader** ppStaticMeshHeaders,
	uint32_t& outNumMeshes)
{
	if (file == nullptr || ppStaticMeshHeaders == nullptr)
	{
		return false;
	}

	if (header.MagicNum != 0x12345678)
	{
		return false;
	}

	const uint32_t& numStaticMesh = header.NumStaticMeshes;
	const uint32_t& staticMeshHeadersOffset = header.MeshHeadersOffset;
	const uint32_t bytesRead = numStaticMesh * sizeof(StaticMeshHeader);

	outNumMeshes = numStaticMesh;

	if (numStaticMesh == 0)
	{
		return true;
	}

	*ppStaticMeshHeaders = (StaticMeshHeader*)malloc(bytesRead);

	file->seekg(staticMeshHeadersOffset);
	file->read((char*)*ppStaticMeshHeaders, bytesRead);

	return true;
}

bool BinaryFileLoader::GetStaticMeshInstances(
	std::fstream* file,
	const ResourceCompilerModule::TileHeader& header,
	ResourceCompilerModule::StaticMeshInstanceHeader** ppStaticMeshInstancesHeaders,
	uint32_t& outNumInstances)
{
	if (file == nullptr || ppStaticMeshInstancesHeaders == nullptr)
	{
		return false;
	}

	if (header.MagicNum != 0x12345678)
	{
		return false;
	}

	const uint32_t& numStaticMeshInstances = header.NumStaticMeshInstances;
	const uint32_t& staticMeshInstanceHeadersOffset = header.StaticMeshInstancesOffset;
	const uint32_t bytesRead = numStaticMeshInstances * sizeof(StaticMeshInstanceHeader);

	outNumInstances = numStaticMeshInstances;
	if (numStaticMeshInstances == 0)
	{
		return true;
	}

	*ppStaticMeshInstancesHeaders = (StaticMeshInstanceHeader*)malloc(bytesRead);

	file->seekg(staticMeshInstanceHeadersOffset);
	file->read((char*)*ppStaticMeshInstancesHeaders, bytesRead);

	return true;
}

bool BinaryFileLoader::GetLightExtensions(
	std::fstream* file,
	const ResourceCompilerModule::TileHeader& header,
	ResourceCompilerModule::LightExtensionHeader** ppLightExtensionHeaders,
	uint32_t& outNumLights)
{
	if (file == nullptr || ppLightExtensionHeaders == nullptr)
	{
		return false;
	}

	if (header.MagicNum != 0x12345678)
	{
		return false;
	}

	const uint32_t& numLightExtensions = header.NumLightExtensions;
	const uint32_t& lightExtensionHeadersOffset = header.LightExtensionsOffset;
	const uint32_t bytesRead = numLightExtensions * sizeof(LightExtensionHeader);

	outNumLights = numLightExtensions;
	if (numLightExtensions == 0)
	{
		return true;
	}

	*ppLightExtensionHeaders = (LightExtensionHeader*)malloc(bytesRead);

	file->seekg(lightExtensionHeadersOffset);
	file->read((char*)*ppLightExtensionHeaders, bytesRead);

	return true;
}

bool BinaryFileLoader::GetMeshParts(
	std::fstream* file,
	const ResourceCompilerModule::TileHeader& header,
	ResourceCompilerModule::MeshPartHeader** ppMeshPartHeaders,
	uint32_t& outMeshParts)
{
	if (file == nullptr || ppMeshPartHeaders == nullptr)
	{
		return false;
	}

	if (header.MagicNum != 0x12345678)
	{
		return false;
	}

	const uint32_t& numMeshParts = header.NumMeshParts;
	const uint32_t& meshPartHeadersOffset = header.MeshPartsOffset;
	const uint32_t bytesRead = numMeshParts * sizeof(MeshPartHeader);

	outMeshParts = numMeshParts;
	if (numMeshParts == 0)
	{
		return true;
	}

	*ppMeshPartHeaders = (MeshPartHeader*)malloc(bytesRead);

	file->seekg(meshPartHeadersOffset);
	file->read((char*)*ppMeshPartHeaders, bytesRead);

	return true;
}