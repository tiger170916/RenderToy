#pragma once
#include "InternalStructs.h"
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

	//bool AddStaticMeshAsset(
	//	std::string tileName,
	//	std::string meshFileName,
	//	std::string meshName);

	bool AddStaticMeshInstance(std::string tileName, std::string meshName, const float position[3], const float rotation[3], const float scale[3]);

	bool AddTileContent(json tileNode, std::string tileName, std::filesystem::path rootDir);

	const std::vector<std::shared_ptr<TileStruct>>& GetTiles() const { return m_tiles; }

	bool PackToBinary(std::filesystem::path rootFilePath);

private:
	bool ParseStaticMeshComponent(json propertiesNode, std::string parentName, TileStruct* tileStruct, SceneObjectComponentInternal*& outSceneObjectComponent);

	bool ParseCameraArmComponent(json propertiesNode, std::string parentName, TileStruct* tileStruct, SceneObjectComponentInternal*& outSceneObjectComponent);

	bool ParseCameraComponent(json propertiesNode, std::string parentName, TileStruct* tileStruct, SceneObjectComponentInternal*& outSceneObjectComponent);

	bool ParseSceneObject(json sceneObjectNode, TileStruct* tileStruct, std::filesystem::path rootDir);

	bool ParsePlayableCharacter(json sceneObjectNode, std::string name, TileStruct* tileStruct);

	bool ParseNonePlayableCharacter(json sceneObjectNode, std::string name, TileStruct* tileStruct);

	bool ParseComponent(json componentNode,  std::string parentName, TileStruct* tileStruct);

	void AddStaticMeshInstances(json instancesNode, StaticMeshComponentInternal* staticMeshStruct);

	void AddLightExtensionToInstance(json lightExtensionNode, StaticMeshInstanceInternal* staticMeshInstanceStruct);

	void PushString(
		const std::string& string,
		std::vector<std::string>& stringArray, 
		std::vector<uint32_t>& startingIdxArray, 
		std::vector<uint32_t>& stringSizesArray,
		uint32_t& itr,
		uint32_t& outIdx);
};