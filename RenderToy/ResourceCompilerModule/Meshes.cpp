#include "Meshes.h"	
#include "FbxLoader.h"


void Meshes::AddStaticMesh(
	std::string name,
	std::string meshAssetPath,
	std::string overrideMaterialName)
{
	StaticMeshDefine* staticMeshDefine = new StaticMeshDefine();
	staticMeshDefine->Name = name;
	staticMeshDefine->MeshAssetPath = meshAssetPath;
	staticMeshDefine->HasOverrideMaterial = !overrideMaterialName.empty();
	if (staticMeshDefine->HasOverrideMaterial)
	{
		staticMeshDefine->OverrideMaterialName = overrideMaterialName;
	}

	m_staticMeshes.push_back(std::shared_ptr<StaticMeshDefine>(staticMeshDefine));
}

void Meshes::AddMaterial(
	std::string name,
	std::string baseColorTexName,
	std::string baseColorTexFile,
	std::string metallicTexName,
	std::string metallicTexFile,
	std::string roughnessTexName,
	std::string roughnessTexFile,
	std::string normalTexName,
	std::string normalTexFile)
{
	MaterialHeader materialHeader = {};
	memcpy(materialHeader.MaterialName, name.c_str(), name.size());

	memcpy(materialHeader.BaseColorTextureName, baseColorTexName.c_str(), baseColorTexName.size());
	memcpy(materialHeader.BaseColorTextureFile, baseColorTexFile.c_str(), baseColorTexFile.size());
	memcpy(materialHeader.MetallicTextureName, metallicTexName.c_str(), metallicTexName.size());
	memcpy(materialHeader.MetallicTextureFile, metallicTexFile.c_str(), metallicTexFile.size());
	memcpy(materialHeader.RoughnessTextureName, roughnessTexName.c_str(), roughnessTexName.size());
	memcpy(materialHeader.RoughnessTextureFile, roughnessTexFile.c_str(), roughnessTexFile.size());
	memcpy(materialHeader.NormalTextureName, normalTexName.c_str(), normalTexName.size());
	memcpy(materialHeader.NormalTextureFile, normalTexFile.c_str(), normalTexFile.size());

	m_materialHeaders.push_back(materialHeader);
}

bool Meshes::PackToBinary(std::filesystem::path rootFilePath)
{
	std::filesystem::path outputDir = rootFilePath.parent_path().append(rootFilePath.stem().string());
	std::filesystem::create_directory(outputDir);

	std::filesystem::path outputFile = outputDir;
	outputFile.append(rootFilePath.stem().string()).concat(".bin");

	std::ofstream file(outputFile, std::ios::out | std::ios::binary);
	if (!file.is_open()) {
		return false;
	}

	std::filesystem::path rootPath = rootFilePath.parent_path();

	std::byte headerBytes[1024] = { };
	// Write binary header
	BinaryHeader binaryHeader = {};
	binaryHeader.BinaryType = BinaryType::BINARY_TYPE_MESHES;
	binaryHeader.MagicNum = 0x12345678;   // 0x12345678 - Magic number  (4 bytes)
	binaryHeader.Version = 0x00000001;	// 0x00000001 - Version       (4 bytes)

	std::vector<std::unique_ptr<FbxLoader>> fbxLoaders;
	std::vector<StaticMeshDefinitionHeader> meshDefHeaders;
	uint32_t meshPartItr = 0;

	int totalNumMeshParts = 0;
	for (auto& staticMesh : m_staticMeshes)
	{
		std::filesystem::path assetFilePath = rootPath;
		assetFilePath.append(staticMesh->MeshAssetPath);

		int overrideMaterialIdx = staticMesh->HasOverrideMaterial ? overrideMaterialIdx = GetMaterialIdxByName(staticMesh->OverrideMaterialName) : -1;
		FbxLoader* fbxLoader = new FbxLoader(assetFilePath, overrideMaterialIdx);

		if (!fbxLoader->Load(true, 0, nullptr))
		{
			delete fbxLoader;
			continue;
		}

		fbxLoaders.push_back(std::unique_ptr<FbxLoader>(fbxLoader));

		totalNumMeshParts += (int)fbxLoader->GetMeshPartHeaders().size();

		StaticMeshDefinitionHeader meshDefHeader = {};
		memcpy(meshDefHeader.MeshName, staticMesh->Name.c_str(), staticMesh->Name.size());
		meshDefHeader.NumParts = (uint32_t)fbxLoader->GetMeshPartHeaders().size();
		meshDefHeader.PartsIdx = meshPartItr;
		meshPartItr += (int)fbxLoader->GetMeshPartHeaders().size();

		meshDefHeaders.push_back(meshDefHeader);
	}

	std::vector<MeshPartHeader> allMeshPartHeaders;
	for (auto& meshLoader : fbxLoaders)
	{
		allMeshPartHeaders.insert(allMeshPartHeaders.end(), meshLoader->GetMeshPartHeaders().begin(), meshLoader->GetMeshPartHeaders().end());
	}

	binaryHeader.NumMeshDefinitions = (uint32_t)meshDefHeaders.size();
	binaryHeader.NumMeshParts = (uint32_t)allMeshPartHeaders.size();
	binaryHeader.NumMaterials = (uint32_t)m_materialHeaders.size();
	binaryHeader.MeshDefinitionsOffset = 1024;
	binaryHeader.MeshPartsOffset = binaryHeader.MeshDefinitionsOffset + (uint32_t)meshDefHeaders.size() * sizeof(StaticMeshDefinitionHeader);
	binaryHeader.MaterialsOffset = binaryHeader.MeshPartsOffset + (uint32_t)allMeshPartHeaders.size() * sizeof(MeshPartHeader);

	uint32_t meshOffset = binaryHeader.MaterialsOffset + (uint32_t)m_materialHeaders.size() * sizeof(MaterialHeader);
	for (auto& meshLoader : fbxLoaders)
	{
		meshLoader->Load(false, &meshOffset, &file);
	}

	memcpy(headerBytes, &binaryHeader, sizeof(BinaryHeader));

	file.seekp(0);
	file.write((char*)headerBytes, 1024);	// Write header
	file.write((char*)meshDefHeaders.data(), meshDefHeaders.size() * sizeof(StaticMeshDefinitionHeader)); // Write mesh definition headers
	file.write((char*)allMeshPartHeaders.data(), allMeshPartHeaders.size() * sizeof(MeshPartHeader));     // Write mesh part headers
	file.write((char*)m_materialHeaders.data(), m_materialHeaders.size() * sizeof(MaterialHeader));       // Write material headers
	file.close();

	return true;
}

int Meshes::GetMaterialIdxByName(std::string name)
{
	for (int i = 0; i < (int)m_materialHeaders.size(); i++)
	{
		if (m_materialHeaders[i].MaterialName == name)
		{
			return i;
		}
	}

	return -1;
}