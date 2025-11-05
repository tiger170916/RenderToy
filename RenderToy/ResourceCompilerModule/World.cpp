#include "World.h"
#include <fstream>
#include "InternalStructs.h"
#include "Utils.h"
#include "FbxLoader.h"

World::World()
{
}

void World::AddTile(std::string tileName, const float bboxMin[2], const float bboxMax[2])
{
	for (auto& tile : m_tiles)
	{
		if (tile->Name == tileName)
		{
			return;
		}
	}
	
	TileStruct* newTile = new TileStruct();
	newTile->Name = tileName;
	newTile->BboxMin[0] = bboxMin[0];
	newTile->BboxMin[1] = bboxMin[1];
	newTile->BboxMax[0] = bboxMax[0];
	newTile->BboxMax[1] = bboxMax[1];

	m_tiles.push_back(std::shared_ptr<TileStruct>(newTile));
}

/*
bool World::AddStaticMeshAsset(
	std::string tileName,
	std::string meshFileName,
	std::string meshName)
{
	TileStruct* foundTile = nullptr;
	for (auto& tile : m_tiles)
	{
		if (tile->Name == tileName)
		{
			foundTile = tile.get();
			break;
		}
	}

	if (foundTile == nullptr)
	{
		return false;
	}

	StaticMeshStruct * staticMeshStruct = new StaticMeshStruct();
	staticMeshStruct->File = meshFileName;
	staticMeshStruct->Name = meshName;

	foundTile->StaticMeshes.push_back(std::shared_ptr<StaticMeshStruct>(staticMeshStruct));

	return true;
}*/

bool World::AddStaticMeshInstance(std::string tileName, std::string meshName, const float position[3], const float rotation[3], const float scale[3])
{
	return true;
}

bool World::AddTileContent(json tileNode, std::string tileName, std::filesystem::path rootDir)
{
	TileStruct* foundStruct = nullptr;
	for (int i = 0; i < m_tiles.size(); i++)
	{
		if (tileName == m_tiles[i]->Name)
		{
			foundStruct = m_tiles[i].get();
			break;
		}
	}

	if (foundStruct == nullptr)
	{
		return false;
	}

	// Get static meshes on this tile.
	if (tileNode.contains("SceneObjects") && tileNode["SceneObjects"].is_array())
	{
		auto sceneObjects = tileNode["SceneObjects"];
		for (auto& sceneObject : sceneObjects)
		{
			ParseSceneObject(sceneObject, foundStruct, rootDir);
		}
	}

	return true;
}

bool World::ParseSceneObject(json sceneObjectNode, TileStruct* tileStruct, std::filesystem::path rootDir)
{
	if (!sceneObjectNode.contains("Name") || !sceneObjectNode.contains("Type") || !sceneObjectNode.contains("RootComponent"))
	{
		return false;
	}

	std::string type = sceneObjectNode["Type"];
	std::string name = sceneObjectNode["Name"];
	if (type == "NonePlayableCharacter")
	{
		return ParseNonePlayableCharacter(sceneObjectNode, name, tileStruct);
	}
	else if (type == "PlayableCharacter")
	{
		return ParsePlayableCharacter(sceneObjectNode, name, tileStruct);
	}

	return false;
}

bool World::ParsePlayableCharacter(json sceneObjectNode, std::string name, TileStruct* tileStruct)
{
	auto& rootComponentNode = sceneObjectNode["RootComponent"];

	// parse root component
	if (ParseComponent(rootComponentNode, name, tileStruct))
	{
		std::shared_ptr<PlayableCharacterInternal> playableObj = std::make_shared<PlayableCharacterInternal>();
		playableObj->Name = name;
		playableObj->RootComponent = rootComponentNode["Name"];

		tileStruct->PlayableObjects.push_back(playableObj);

		return true;
	}

	return false;
}

bool World::ParseNonePlayableCharacter(json sceneObjectNode, std::string name, TileStruct* tileStruct)
{
	auto& rootComponentNode = sceneObjectNode["RootComponent"];

	// parse root component
	if (ParseComponent(rootComponentNode, name, tileStruct))
	{
		std::shared_ptr<NonePlayableCharacterInternal> nonePlayableObj = std::make_shared<NonePlayableCharacterInternal>();
		nonePlayableObj->Name = name;
		nonePlayableObj->RootComponent = rootComponentNode["Name"];

		tileStruct->NonePlayableObjects.push_back(nonePlayableObj);

		return true;
	}

	return false;
}

bool World::ParseComponent(json componentNode, std::string parentName, TileStruct* tileStruct)
{
	if (!componentNode.contains("Type") || !componentNode.contains("Properties"))
	{
		return false;
	}

	std::string type = componentNode["Type"];
	std::string name = componentNode["Name"];

	SceneObjectComponentInternal* outSceneObjectComponent = nullptr;
	if (type == "Mesh")
	{
		World::ParseStaticMeshComponent(componentNode, parentName, tileStruct, outSceneObjectComponent);
	}
	else if (type == "CameraArm")
	{
		World::ParseCameraArmComponent(componentNode, parentName, tileStruct, outSceneObjectComponent);
	}
	else if (type == "Camera")
	{
		World::ParseCameraComponent(componentNode, parentName, tileStruct, outSceneObjectComponent);
	}

	if (outSceneObjectComponent)
	{
		// Parse child components
		if (componentNode.contains("Components") && componentNode["Components"].is_array())
		{
			auto childComponentsNode = componentNode["Components"];
			for (auto& childComponentNode : childComponentsNode)
			{
				if (ParseComponent(childComponentNode, name, tileStruct))
				{
					outSceneObjectComponent->Components.push_back(childComponentNode["Name"]);
				}
			}
		}
	}

	return outSceneObjectComponent != nullptr;
}

bool World::ParseCameraArmComponent(json componentNode, std::string parentName, TileStruct* tileStruct, SceneObjectComponentInternal*& outSceneObjectComponent)
{
	outSceneObjectComponent = nullptr;
	if (!componentNode.contains("Name") || !componentNode.contains("Properties"))
	{
		return false;
	}

	auto& properties = componentNode["Properties"];
	if (!properties.contains("Length"))
	{
		return false;
	}

	float armLen = properties["Length"];

	std::shared_ptr<CameraArmComponentInternal> cameraArmComponent = std::make_shared<CameraArmComponentInternal>();
	
	cameraArmComponent->Name = componentNode["Name"];
	cameraArmComponent->Parent = parentName;
	cameraArmComponent->Length = armLen;

	tileStruct->CameraArmComponents.push_back(cameraArmComponent);

	outSceneObjectComponent = cameraArmComponent.get();
	return true;
}

bool World::ParseCameraComponent(json componentNode, std::string parentName, TileStruct* tileStruct, SceneObjectComponentInternal*& outSceneObjectComponent)
{
	outSceneObjectComponent = nullptr;

	if (!componentNode.contains("Name") || !componentNode.contains("Properties"))
	{
		return false;
	}

	std::shared_ptr<CameraComponentInternal> cameraComponent = std::make_shared<CameraComponentInternal>();
	cameraComponent->Name = componentNode["Name"];
	cameraComponent->Parent = parentName;

	tileStruct->CameraComponents.push_back(cameraComponent);

	outSceneObjectComponent = cameraComponent.get();
	return true;
}

bool World::ParseStaticMeshComponent(json componentNode, std::string parentName, TileStruct* tileStruct, SceneObjectComponentInternal*& outSceneObjectComponent)
{
	outSceneObjectComponent = nullptr;
	if (!componentNode.contains("Name") || !componentNode.contains("Properties"))
	{
		return false;
	}

	auto& propertiesNode = componentNode["Properties"];

	StaticMeshComponentInternal* staticMeshComponent = new StaticMeshComponentInternal();

	// Get mesh info
	staticMeshComponent->File = propertiesNode["File"];
	staticMeshComponent->Name = componentNode["Name"];
	staticMeshComponent->Parent = parentName;
	
	tileStruct->StaticMeshComponents.push_back(std::shared_ptr<StaticMeshComponentInternal>(staticMeshComponent));
	if (propertiesNode.contains("Instances") && propertiesNode["Instances"].is_array())
	{
		auto instancesNode = propertiesNode["Instances"];
		AddStaticMeshInstances(instancesNode, staticMeshComponent);
	}

	outSceneObjectComponent = staticMeshComponent;
	return true;
}

void World::AddStaticMeshInstances(json instancesNode, StaticMeshComponentInternal* staticMeshStruct)
{
	for (auto& instanceNode : instancesNode)
	{
		if (!instanceNode.is_object())
		{
			continue;
		}

		if (!instanceNode.contains("Position") || !instanceNode.contains("Rotation") || !instanceNode.contains("Scale"))
		{
			continue;
		}

		StaticMeshInstanceInternal* instanceStruct = new StaticMeshInstanceInternal();
		auto positionNode = instanceNode["Position"];
		auto rotationNode = instanceNode["Rotation"];
		auto scaleNode = instanceNode["Scale"];

		instanceStruct->Position[0] = positionNode[0];
		instanceStruct->Position[1] = positionNode[1];
		instanceStruct->Position[2] = positionNode[2];

		instanceStruct->Rotation[0] = rotationNode[0];
		instanceStruct->Rotation[1] = rotationNode[1];
		instanceStruct->Rotation[2] = rotationNode[2];

		instanceStruct->Scale[0] = scaleNode[0];
		instanceStruct->Scale[1] = scaleNode[1];
		instanceStruct->Scale[2] = scaleNode[2];

		staticMeshStruct->Instances.push_back(std::shared_ptr<StaticMeshInstanceInternal>(instanceStruct));

		if (instanceNode.contains("LightExtension") && instanceNode["LightExtension"].is_object())
		{
			auto lightExtensionNode = instanceNode["LightExtension"];
			AddLightExtensionToInstance(lightExtensionNode, instanceStruct);
		}
	}
}

void World::AddLightExtensionToInstance(json lightExtensionNode, StaticMeshInstanceInternal* staticMeshInstanceStruct)
{
	LightExtensionInternal* lightExt = new LightExtensionInternal();
	staticMeshInstanceStruct->LightExt = std::shared_ptr<LightExtensionInternal>(lightExt);
	lightExt->Offset[0] = lightExtensionNode["Offset"][0];
	lightExt->Offset[1] = lightExtensionNode["Offset"][1];
	lightExt->Offset[2] = lightExtensionNode["Offset"][2];
	lightExt->Color[0] = lightExtensionNode["Color"][0];
	lightExt->Color[1] = lightExtensionNode["Color"][1];
	lightExt->Color[2] = lightExtensionNode["Color"][2];
	lightExt->Rotation[0] = lightExtensionNode["Rotation"][0];
	lightExt->Rotation[1] = lightExtensionNode["Rotation"][1];
	lightExt->Rotation[2] = lightExtensionNode["Rotation"][2];
	lightExt->AspectRatio = lightExtensionNode["AspectRatio"];
	lightExt->Intensity = lightExtensionNode["Intensity"];
	lightExt->Fov = lightExtensionNode["Fov"];
	lightExt->AttenuationRadius = lightExtensionNode["AttenuationRadius"];
	lightExt->LightType = lightExtensionNode["LightType"];
}

bool World::PackToBinary(std::filesystem::path rootFilePath)
{
	// Create output dir if not existed
	
	std::filesystem::path outputDir = rootFilePath.parent_path().append(rootFilePath.stem().string());
	std::filesystem::create_directory(outputDir);

	for (auto& tile : m_tiles)
	{
		if (tile->NonePlayableObjects.empty() && tile->PlayableObjects.empty())
		{
			continue;
		}

		std::filesystem::path outputFile = rootFilePath.parent_path().append(rootFilePath.stem().string()).append(tile->Name).concat(".bin");

		std::ofstream file(outputFile, std::ios::out | std::ios::binary);
		if (!file.is_open()) {
			return false;
		}

		// Write binary header
		BinaryHeader binaryHeader = {};
		binaryHeader.BinaryType = BinaryType::BINARY_TYPE_TILE;
		binaryHeader.MagicNum = 0x12345678;   // 0x12345678 - Magic number  (4 bytes)
		binaryHeader.Version = 0x00000001;	// 0x00000001 - Version       (4 bytes)
		memcpy(binaryHeader.TileName, tile->Name.c_str(), tile->Name.size());
		binaryHeader.BboxMinX = tile->BboxMin[0];	// BboxMin 8 bytes
		binaryHeader.BboxMinY = tile->BboxMin[1];
		binaryHeader.BboxMaxX = tile->BboxMax[0];	// Bboxmax 8 bytes
		binaryHeader.BboxMaxY = tile->BboxMax[1];

		std::vector<std::string> stringTable;
		std::vector<uint32_t> stringStartingIndices;
		std::vector<uint32_t> stringSizes;
		uint32_t currentStringPos = 0;
 
		// Write none playable scene objects
		std::vector<NonePlayableCharacterHeader> nonePlayableCharacterHeaders;
		for (auto& nonePlayableCharacter : tile->NonePlayableObjects)
		{
			NonePlayableCharacterHeader nonePlayableHeader = {};
			
			PushString(nonePlayableCharacter->Name, stringTable, stringStartingIndices, stringSizes, currentStringPos, nonePlayableHeader.NameStringIdx);
			PushString(nonePlayableCharacter->RootComponent, stringTable, stringStartingIndices, stringSizes, currentStringPos, nonePlayableHeader.RootComponentNameIdx);
			nonePlayableCharacterHeaders.push_back(nonePlayableHeader);
		}

		// Write playable scene objects
		std::vector<PlayableCharacterHeader> playableCharacterHeaders;
		for (auto& playableCharacter : tile->NonePlayableObjects)
		{
			PlayableCharacterHeader playableHeader = {};

			PushString(playableCharacter->Name, stringTable, stringStartingIndices, stringSizes, currentStringPos, playableHeader.NameStringIdx);
			PushString(playableCharacter->RootComponent, stringTable, stringStartingIndices, stringSizes, currentStringPos, playableHeader.RootComponentNameIdx);
			playableCharacterHeaders.push_back(playableHeader);
		}

		// Write static mesh objects
		std::vector<StaticMeshComponentHeader> staticMeshComponentHeaders;
		std::vector<StaticMeshInstanceHeader> staticMeshInstanceHeaders;	// static mesh instances
		std::vector<LightExtensionHeader> lightExtensionHeaders;			// lights
		uint32_t lightExtensionIndex = 0;
		uint32_t staticMeshInstanceIndex = 0;

		for (auto& staticMesh : tile->StaticMeshComponents)
		{
			StaticMeshComponentHeader staticMeshHeader = {};
			PushString(staticMesh->Name, stringTable, stringStartingIndices, stringSizes, currentStringPos, staticMeshHeader.NameStringIdx);
			PushString(staticMesh->Parent, stringTable, stringStartingIndices, stringSizes, currentStringPos, staticMeshHeader.ParentNameStringIdx);
			PushString(staticMesh->File, stringTable, stringStartingIndices, stringSizes, currentStringPos, staticMeshHeader.FileNameStringIdx);

			// Push component names
			if (!staticMesh->Components.empty())
			{
				staticMeshHeader.ComponentsNameStartingIdx = (uint32_t)stringTable.size();
				staticMeshHeader.NumComponents = (uint32_t)staticMesh->Components.size();
				for (auto& componentName : staticMesh->Components)
				{
					uint32_t strIdx = 0;
					PushString(componentName, stringTable, stringStartingIndices, stringSizes, currentStringPos, strIdx);
				}
			}

			// Push static mesh instances
			for (size_t j = 0; j < staticMesh->Instances.size(); j++)
			{
				StaticMeshInstanceHeader staticMeshInstanceHeader = {};
				StaticMeshInstanceInternal* staticMeshInstance = staticMesh->Instances[j].get();
				memcpy(staticMeshInstanceHeader.Position, staticMeshInstance->Position, 3 * sizeof(float));
				memcpy(staticMeshInstanceHeader.Rotation, staticMeshInstance->Rotation, 3 * sizeof(float));
				memcpy(staticMeshInstanceHeader.Scale, staticMeshInstance->Scale, 3 * sizeof(float));
				staticMeshInstanceHeader.HasLightExtension = staticMeshInstance->LightExt != nullptr;
				staticMeshInstanceHeader.LightExtensionIndex = staticMeshInstance->LightExt ? lightExtensionIndex : 0;

				if (staticMeshInstance->LightExt != nullptr)
				{
					LightExtensionInternal* lightExt = staticMeshInstance->LightExt.get();
					LightType lightType = Utils::GetLightTypeFromString(lightExt->LightType);
					if (lightType == LightType::NONE)
					{
						continue;
					}

					LightExtensionHeader lightExtHeader = {};
					lightExtHeader.AspectRatio = lightExt->AspectRatio;
					lightExtHeader.AttenuationRadius = lightExt->AttenuationRadius;
					memcpy(lightExtHeader.Color, lightExt->Color, 3 * sizeof(float));
					lightExtHeader.Fov = lightExt->Fov;
					lightExtHeader.Intensity = lightExt->Intensity;
					memcpy(lightExtHeader.Offset, lightExt->Offset, 3 * sizeof(float));
					memcpy(lightExtHeader.Rotation, lightExt->Rotation, 3 * sizeof(float));
					lightExtHeader.LightType = (uint32_t)lightType;

					lightExtensionHeaders.push_back(lightExtHeader);

					lightExtensionIndex += 1;
				}

				staticMeshInstanceHeaders.push_back(staticMeshInstanceHeader);
			}

			staticMeshHeader.InstanceCount = (uint32_t)staticMesh->Instances.size();
			staticMeshHeader.InstanceIndex = staticMeshInstanceIndex;

			staticMeshComponentHeaders.push_back(staticMeshHeader);

			staticMeshInstanceIndex += (uint32_t)staticMesh->Instances.size();
		}

		// Push camera arms
		std::vector<CameraArmComponentHeader> cameraArmComponentHeaders;
		for (auto& cameraArm : tile->CameraArmComponents)
		{
			CameraArmComponentHeader cameraArmHeader = {};
			PushString(cameraArm->Name, stringTable, stringStartingIndices, stringSizes, currentStringPos, cameraArmHeader.NameStringIdx);
			PushString(cameraArm->Parent, stringTable, stringStartingIndices, stringSizes, currentStringPos, cameraArmHeader.ParentNameStringIdx);

			// Push component names
			if (!cameraArm->Components.empty())
			{
				cameraArmHeader.ComponentsNameStartingIdx = (uint32_t)stringTable.size();
				cameraArmHeader.NumComponents = (uint32_t)cameraArm->Components.size();
				for (auto& componentName : cameraArm->Components)
				{
					uint32_t strIdx = 0;
					PushString(componentName, stringTable, stringStartingIndices, stringSizes, currentStringPos, strIdx);
				}
			}

			cameraArmHeader.Length = cameraArm->Length;
			
			cameraArmComponentHeaders.push_back(cameraArmHeader);
		}

		// Push cameras
		std::vector<CameraComponentHeader> cameraComponentHeaders;
		for (auto& camera : tile->CameraComponents)
		{
			CameraComponentHeader cameraHeader = {};
			PushString(camera->Name, stringTable, stringStartingIndices, stringSizes, currentStringPos, cameraHeader.NameStringIdx);
			PushString(camera->Parent, stringTable, stringStartingIndices, stringSizes, currentStringPos, cameraHeader.ParentNameStringIdx);

			// Push component names
			if (!camera->Components.empty())
			{
				cameraHeader.ComponentsNameStartingIdx = (uint32_t)stringTable.size();
				cameraHeader.NumComponents = (uint32_t)camera->Components.size();
				for (auto& componentName : camera->Components)
				{
					uint32_t strIdx = 0;
					PushString(componentName, stringTable, stringStartingIndices, stringSizes, currentStringPos, strIdx);
				}

				cameraComponentHeaders.push_back(cameraHeader);
			}
		}



		//uint32_t meshHeadersOffset = 1024;
		//uint32_t staticMeshInstancesOffset = meshHeadersOffset + (uint32_t)staticMeshHeaders.size() * sizeof(StaticMeshHeader);
		//uint32_t lightExtensionsOffset = staticMeshInstancesOffset + (uint32_t)staticMeshInstanceHeaders.size() * sizeof(StaticMeshInstanceHeader);
		//uint32_t stringsOffset = lightExtensionsOffset + (uint32_t)lightExtensionHeaders.size() * sizeof(LightExtensionHeader);

		uint32_t offset = 1024;

		// Figure sizes and offsets
		// Playable characters
		binaryHeader.PlayableCharactersOffset = offset;
		binaryHeader.NumPlayableCharacters = (uint32_t)playableCharacterHeaders.size();
		offset += binaryHeader.NumPlayableCharacters * sizeof(PlayableCharacterHeader);

		// None playable characters
		binaryHeader.NonePlayableCharactersOffset = offset;
		binaryHeader.NumNonePlayableCharacters = (uint32_t)nonePlayableCharacterHeaders.size();
		offset += binaryHeader.NumNonePlayableCharacters * sizeof(NonePlayableCharacterHeader);

		// StaticMesh components
		binaryHeader.StaticMeshComponentsOffset = offset;
		binaryHeader.NumStaticMeshComponents = (uint32_t)staticMeshComponentHeaders.size();
		offset += binaryHeader.NumStaticMeshComponents * sizeof(StaticMeshComponentHeader);

		// Camera arm components
		binaryHeader.CameraArmComponentsOffset = offset;
		binaryHeader.NumCameraArmComponents = (uint32_t)cameraArmComponentHeaders.size();
		offset += binaryHeader.NumCameraArmComponents * sizeof(CameraArmComponentHeader);

		// camera components
		binaryHeader.CameraComponentsOffset = offset;
		binaryHeader.NumCameraComponents = (uint32_t)cameraComponentHeaders.size();
		offset += binaryHeader.NumCameraComponents * sizeof(CameraComponentHeader);

		// mesh instances
		binaryHeader.StaticMeshInstancesOffset = offset;
		binaryHeader.NumStaticMeshInstances = (uint32_t)staticMeshInstanceHeaders.size();
		offset += binaryHeader.NumStaticMeshInstances * sizeof(StaticMeshInstanceHeader);

		// light extensions
		binaryHeader.LightExtensionsOffset = offset;
		binaryHeader.NumLightExtensions = (uint32_t)lightExtensionHeaders.size();
		offset += binaryHeader.NumLightExtensions * sizeof(LightExtensionHeader);

		// string section
		binaryHeader.NumStrings = (uint32_t)stringTable.size();
		binaryHeader.StringStartingPositionsOffset = offset;
		offset += binaryHeader.NumStrings * sizeof(uint32_t);

		binaryHeader.StringSizesOffset = offset;
		offset += binaryHeader.NumStrings * sizeof(uint32_t);

		uint32_t totalStrLen = 0;
		for (auto& str : stringTable)
		{
			totalStrLen += (uint32_t)str.length();
		}
		binaryHeader.StringSectionOffset = offset;
		binaryHeader.StringSectionSize = totalStrLen;



		file.seekp(0);
		// Write binary header
		file.write((char*)&binaryHeader, 1024);
		// Write playable character headers
		file.write((char*)playableCharacterHeaders.data(), sizeof(PlayableCharacterHeader) * binaryHeader.NumPlayableCharacters);
		// Write none playable character headers
		file.write((char*)nonePlayableCharacterHeaders.data(), sizeof(NonePlayableCharacterHeader) * binaryHeader.NumNonePlayableCharacters);
		// Write static mesh component headers
		file.write((char*)staticMeshComponentHeaders.data(), sizeof(StaticMeshComponentHeader) * binaryHeader.NumStaticMeshComponents);
		// Write camera arm component headers
		file.write((char*)cameraArmComponentHeaders.data(), sizeof(CameraArmComponentHeader) * binaryHeader.NumCameraArmComponents);
		// Write camera component headers
		file.write((char*)cameraComponentHeaders.data(), sizeof(CameraComponentHeader) * binaryHeader.NumCameraComponents);
		// Write mesh instance headers
		file.write((char*)staticMeshInstanceHeaders.data(), sizeof(StaticMeshInstanceHeader) * binaryHeader.NumStaticMeshInstances);
		// Write light extension headers
		file.write((char*)lightExtensionHeaders.data(), sizeof(LightExtensionHeader) * binaryHeader.NumLightExtensions);
		// Write string section
		file.write((char*)stringStartingIndices.data(), sizeof(uint32_t)* binaryHeader.NumStrings);
		file.write((char*)stringSizes.data(), sizeof(uint32_t)* binaryHeader.NumStrings);
		// write actual string
		for (auto& str : stringTable)
		{
			file.write(str.c_str(), str.length());
		}

		file.close();
	}

	return true;
}

void World::PushString(
	const std::string& string,
	std::vector<std::string>& stringArray,
	std::vector<uint32_t>& startingIdxArray,
	std::vector<uint32_t>& stringSizesArray,
	uint32_t& itr,
	uint32_t& outIdx)
{
	outIdx = (uint32_t)stringArray.size();
	stringArray.push_back(string);
	startingIdxArray.push_back(itr);
	stringSizesArray.push_back((uint32_t)string.size());
	itr += (uint32_t)string.size();
}