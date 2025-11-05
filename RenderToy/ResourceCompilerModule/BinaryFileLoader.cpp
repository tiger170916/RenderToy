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
	BinaryHeader* pHeader = (BinaryHeader*)header;

	if (pHeader->MagicNum != 0x12345678)
	{
		// Wrong file format
		return false;
	}

	std::vector<StaticMeshHeader> staticMeshHeaders(pHeader->NumStaticMeshes);
	std::vector<StaticMeshInstanceHeader> staticMeshInstanceHeaders(pHeader->NumStaticMeshInstances);
	std::vector<LightExtensionHeader> lightExtensionHeaders(pHeader->NumLightExtensions);

	file.read((char*)staticMeshHeaders.data(), pHeader->NumStaticMeshes * sizeof(StaticMeshHeader));
	file.read((char*)staticMeshInstanceHeaders.data(), pHeader->NumStaticMeshInstances * sizeof(StaticMeshInstanceHeader));
	file.read((char*)lightExtensionHeaders.data(), pHeader->NumLightExtensions * sizeof(LightExtensionHeader));

	file.close();

	return true;
}

bool BinaryFileLoader::ReadHeader(std::fstream* file, ResourceCompilerModule::BinaryHeader& header)
{
	if (!file)
	{
		return false;
	}
	 
	std::byte headerBytes[1024];
	file->seekg(0);
	file->read((char*)headerBytes, 1024);
	BinaryHeader* pheader = (BinaryHeader*)headerBytes;

	if (pheader->MagicNum != 0x12345678)
	{
		// Wrong file format
		return false;
	}

	memcpy(&header, headerBytes, sizeof(BinaryHeader));

	return true;
}

bool BinaryFileLoader::GetStaticMeshes(
	std::fstream* file,
	const ResourceCompilerModule::BinaryHeader& header,
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

	if (header.BinaryType != BinaryType::BINARY_TYPE_TILE)
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
	const ResourceCompilerModule::BinaryHeader& header,
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

	if (header.BinaryType != BinaryType::BINARY_TYPE_TILE)
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
	const ResourceCompilerModule::BinaryHeader& header,
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

	if (header.BinaryType != BinaryType::BINARY_TYPE_TILE)
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

bool BinaryFileLoader::GetStaticMeshDefinitions(
	std::fstream* file,
	const ResourceCompilerModule::BinaryHeader& header,
	ResourceCompilerModule::StaticMeshDefinitionHeader** ppStaticMeshDefinitionHeaders,
	uint32_t& outNumStaticMeshDefinitions)
{
	if (file == nullptr || ppStaticMeshDefinitionHeaders == nullptr)
	{
		return false;
	}
	if (header.MagicNum != 0x12345678)
	{
		return false;
	}

	if (header.BinaryType != BinaryType::BINARY_TYPE_MESHES)
	{
		return false;
	}

	const uint32_t& numStaticMeshDefinitions = header.NumMeshDefinitions;
	const uint32_t& staticMeshDefinitionHeadersOffset = header.MeshDefinitionsOffset;
	const uint32_t bytesRead = numStaticMeshDefinitions * sizeof(StaticMeshDefinitionHeader);
	outNumStaticMeshDefinitions = numStaticMeshDefinitions;
	if (numStaticMeshDefinitions == 0)
	{
		return true;
	}
	*ppStaticMeshDefinitionHeaders = (StaticMeshDefinitionHeader*)malloc(bytesRead);
	file->seekg(staticMeshDefinitionHeadersOffset);
	file->read((char*)*ppStaticMeshDefinitionHeaders, bytesRead);
	return true;
}

bool BinaryFileLoader::GetMeshPartHeaders(
	std::fstream* file,
	const ResourceCompilerModule::BinaryHeader& header,
	ResourceCompilerModule::MeshPartHeader** ppMeshPartHeaders,
	uint32_t& outNumMeshPartHeaders)
{
	if (file == nullptr || ppMeshPartHeaders == nullptr)
	{
		return false;
	}
	if (header.MagicNum != 0x12345678)
	{
		return false;
	}

	if (header.BinaryType != BinaryType::BINARY_TYPE_MESHES)
	{
		return false;
	}

	const uint32_t& numMeshPartHeaders = header.NumMeshParts;
	const uint32_t& meshPartHeadersOffset = header.MeshPartsOffset;
	const uint32_t bytesRead = numMeshPartHeaders * sizeof(MeshPartHeader);
	outNumMeshPartHeaders = numMeshPartHeaders;
	if (numMeshPartHeaders == 0)
	{
		return true;
	}
	*ppMeshPartHeaders = (MeshPartHeader*)malloc(bytesRead);
	file->seekg(meshPartHeadersOffset);
	file->read((char*)*ppMeshPartHeaders, bytesRead);
	return true;
}

bool BinaryFileLoader::GetMaterialHeaders(
	std::fstream* file,
	const ResourceCompilerModule::BinaryHeader& header,
	ResourceCompilerModule::MaterialHeader** ppMaterialHeaders,
	uint32_t& outNumMaterialHeaders)
{
	if (file == nullptr || ppMaterialHeaders == nullptr)
	{
		return false;
	}
	if (header.MagicNum != 0x12345678)
	{
		return false;
	}
	if (header.BinaryType != BinaryType::BINARY_TYPE_MESHES)
	{
		return false;
	}
	const uint32_t& numMaterialHeaders = header.NumMaterials;
	const uint32_t& materialHeadersOffset = header.MaterialsOffset;
	const uint32_t bytesRead = numMaterialHeaders * sizeof(MaterialHeader);
	outNumMaterialHeaders = numMaterialHeaders;
	if (numMaterialHeaders == 0)
	{
		return true;
	}
	*ppMaterialHeaders = (MaterialHeader*)malloc(bytesRead);
	file->seekg(materialHeadersOffset);
	file->read((char*)*ppMaterialHeaders, bytesRead);

	return true;
}

bool BinaryFileLoader::GetTextureHeaders(
	std::fstream* file,
	const ResourceCompilerModule::BinaryHeader& header,
	ResourceCompilerModule::TextureHeader** ppTextureHeaders,
	uint32_t& outNumTextureHeaders)
{
	if (file == nullptr || ppTextureHeaders == nullptr)
	{
		return false;
	}
	if (header.MagicNum != 0x12345678)
	{
		return false;
	}
	if (header.BinaryType != BinaryType::BINARY_TYPE_TEXTURES)
	{
		return false;
	}

	const uint32_t& numTextureHeaders = header.NumTextures;
	const uint32_t& textureHeadersOffset = header.TexturesOffset;
	const uint32_t bytesRead = numTextureHeaders * sizeof(TextureHeader);
	outNumTextureHeaders = numTextureHeaders;
	if (numTextureHeaders == 0)
	{
		return true;
	}

	*ppTextureHeaders = (TextureHeader*)malloc(bytesRead);
	file->seekg(textureHeadersOffset);
	file->read((char*)*ppTextureHeaders, bytesRead);

	return true;
}