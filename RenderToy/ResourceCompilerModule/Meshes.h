#pragma once

#include <set>
#include <vector>
#include <filesystem>
#include "3rdParty/json.hpp"
#include "Structs.h"
#include "ResourceStructs.h"

using json = nlohmann::json;
using namespace ResourceCompilerModule;


class Meshes
{
private:
	std::vector<std::shared_ptr<StaticMeshDefine>> m_staticMeshes;

	//std::vector<MaterialHeader> m_materials; // all materials
	std::vector<MaterialHeader> m_materialHeaders;

public:
	void AddStaticMesh(
		std::string name,
		std::string meshAssetPath,
		std::string overrideMaterialName = std::string());

	void AddMaterial(
		std::string name,
		std::string baseColorTexName,
		std::string baseColorTexFile,
		std::string metallicTexName,
		std::string metallicTexFile,
		std::string roughnessTexName,
		std::string roughnessTexFile,
		std::string normalTexName,
		std::string normalTexFile);

	bool PackToBinary(std::filesystem::path rootFilePath);

private:
	int GetMaterialIdxByName(std::string name);
};