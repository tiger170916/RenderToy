#pragma once
#include "Structs.h"
#include <set>
#include <vector>
#include <filesystem>
#include "3rdParty/json.hpp"

using json = nlohmann::json;


class World
{
private:
	std::vector<std::shared_ptr<TileStruct>> m_tiles;

public:
	World();

	void AddTile(std::string tileName, const float bboxMin[2], const float bboxMax[2]);

	bool AddStaticMeshAsset(
		std::string tileName,
		std::string meshFileName,
		std::string meshName);

	bool AddStaticMeshInstance(std::string tileName, std::string meshName, const float position[3], const float rotation[3], const float scale[3]);

	bool AddTileContent(json tileNode, std::string tileName, std::filesystem::path rootDir);

	const std::vector<std::shared_ptr<TileStruct>>& GetTiles() const { return m_tiles; }

	bool PackToBinary(std::filesystem::path rootFilePath);

private:
	bool AddStaticMesh(json staticMeshNode, TileStruct* tileStruct, std::filesystem::path rootDir);

	void AddStaticMeshInstances(json instancesNode, StaticMeshStruct* staticMeshStruct);

	void AddLightExtensionToInstance(json lightExtensionNode, StaticMeshInstanceStruct* staticMeshInstanceStruct);
};