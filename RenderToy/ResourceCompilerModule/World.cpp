#include "World.h"
#include <fstream>
#include "ResourceStructs.h"
#include "Utils.h"
#include "FbxLoader.h"

using namespace ResourceCompilerModule;

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

bool World::AddStaticMeshAsset(
	std::string tileName,
	std::string meshName,
	std::filesystem::path assetPath,
	std::filesystem::path overrideNormalTexture,
	std::filesystem::path overrideMetallicTexture,
	std::filesystem::path overrideRoughnessTexture,
	std::filesystem::path overrideBaseColorTexture)
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
	staticMeshStruct->MeshName = meshName;

	staticMeshStruct->AssetPath = assetPath;
	staticMeshStruct->OverrideNormalTexture = overrideNormalTexture;
	staticMeshStruct->OverrideMetallicTexture = overrideMetallicTexture;
	staticMeshStruct->OverrideRoughnessTexture = overrideRoughnessTexture;
	staticMeshStruct->OverrideBaseColorTexture = overrideBaseColorTexture;

	foundTile->StaticMeshes.push_back(std::shared_ptr<StaticMeshStruct>(staticMeshStruct));

	return true;
}

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
	if (tileNode.contains("StaticMeshes") && tileNode["StaticMeshes"].is_array())
	{
		auto staticMeshes = tileNode["StaticMeshes"];
		for (auto& staticMesh : staticMeshes)
		{
			AddStaticMesh(staticMesh, foundStruct, rootDir);
		}
	}

	return true;
}

bool World::AddStaticMesh(json staticMeshNode, TileStruct* tileStruct, std::filesystem::path rootDir)
{
	if (!staticMeshNode.contains("Name") || !staticMeshNode.contains("MeshAsset"))
	{
		return false;
	}

	StaticMeshStruct* staticMeshStruct = new StaticMeshStruct();

	// Get mesh info
	staticMeshStruct->MeshName = staticMeshNode["Name"];
	std::string meshAsset = staticMeshNode["MeshAsset"];
	std::filesystem::path meshAssetPath = rootDir;
	meshAssetPath.append(meshAsset);
	staticMeshStruct->AssetPath = meshAssetPath;

	std::filesystem::path normalOverrideTexture;
	std::filesystem::path metallicOverrideTexture;
	std::filesystem::path roughnessOverrideTexture;
	std::filesystem::path baseColorOverrideTexture;

	if (staticMeshNode.contains("TextureOverride"))
	{
		auto overrideTextures = staticMeshNode["TextureOverride"];
		if (overrideTextures.contains("Normal"))
		{
			staticMeshStruct->OverrideNormalTexture = rootDir;
			staticMeshStruct->OverrideNormalTexture.append(overrideTextures["Normal"].get<std::string>());
		}
		if (overrideTextures.contains("Metallic"))
		{
			staticMeshStruct->OverrideMetallicTexture = rootDir;
			staticMeshStruct->OverrideMetallicTexture.append(overrideTextures["Metallic"].get<std::string>());
		}
		if (overrideTextures.contains("Roughness"))
		{
			staticMeshStruct->OverrideRoughnessTexture = rootDir;
			staticMeshStruct->OverrideRoughnessTexture .append(overrideTextures["Roughness"].get<std::string>());
		}
		if (overrideTextures.contains("BaseColor"))
		{
			staticMeshStruct->OverrideBaseColorTexture = rootDir;
			staticMeshStruct->OverrideBaseColorTexture.append(overrideTextures["BaseColor"].get<std::string>());
		}
	}

	tileStruct->StaticMeshes.push_back(std::shared_ptr<StaticMeshStruct>(staticMeshStruct));
	if (staticMeshNode.contains("Instances") && staticMeshNode["Instances"].is_array())
	{
		auto instancesNode = staticMeshNode["Instances"];
		AddStaticMeshInstances(instancesNode, staticMeshStruct);
	}

	return true;
}

void World::AddStaticMeshInstances(json instancesNode, StaticMeshStruct* staticMeshStruct)
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

		StaticMeshInstanceStruct* instanceStruct = new StaticMeshInstanceStruct();
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

		staticMeshStruct->Instances.push_back(std::shared_ptr<StaticMeshInstanceStruct>(instanceStruct));

		if (instanceNode.contains("LightExtension") && instanceNode["LightExtension"].is_object())
		{
			auto lightExtensionNode = instanceNode["LightExtension"];
			AddLightExtensionToInstance(lightExtensionNode, instanceStruct);
		}
	}
}

void World::AddLightExtensionToInstance(json lightExtensionNode, StaticMeshInstanceStruct* staticMeshInstanceStruct)
{
	LightExtension* lightExt = new LightExtension();
	staticMeshInstanceStruct->LightExt = std::shared_ptr<LightExtension>(lightExt);
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
		if (tile->StaticMeshes.empty())
		{
			continue;
		}

		std::filesystem::path outputFile = rootFilePath.parent_path().append(rootFilePath.stem().string()).append(tile->Name).concat(".bin");

		std::ofstream file(outputFile, std::ios::out | std::ios::binary);
		if (!file.is_open()) {
			return false;
		}

		std::byte headerBytes[1024] = { };
		// Write binary header
		TileHeader tileHeader = {};
		tileHeader.MagicNum = 0x12345678;   // 0x12345678 - Magic number  (4 bytes)
		tileHeader.Version = 0x00000001;	// 0x00000001 - Version       (4 bytes)
		memcpy(tileHeader.TileName, tile->Name.c_str(), tile->Name.size());
		tileHeader.BboxMinX = tile->BboxMin[0];	// BboxMin 8 bytes
		tileHeader.BboxMinY = tile->BboxMin[1];
		tileHeader.BboxMaxX = tile->BboxMax[0];	// Bboxmax 8 bytes
		tileHeader.BboxMaxY = tile->BboxMax[1];

		// Write static mesh headers
		std::vector<StaticMeshHeader> staticMeshHeaders;
		std::vector<StaticMeshInstanceHeader> staticMeshInstanceHeaders;
		std::vector<LightExtensionHeader> lightExtensionHeaders;
		uint32_t stringItr = 0;

		stringItr += ((uint32_t)tile->Name.size() + 1);

		uint32_t lightExtensionIndex = 0;
		uint32_t staticMeshInstanceIndex = 0;
		for (size_t i = 0; i < tile->StaticMeshes.size(); i++)
		{
			StaticMeshStruct* staticMesh = tile->StaticMeshes[i].get();

			for (size_t j = 0; j < staticMesh->Instances.size(); j++)
			{
				StaticMeshInstanceHeader staticMeshInstanceHeader = {};
				StaticMeshInstanceStruct* staticMeshInstance = staticMesh->Instances[j].get();
				memcpy(staticMeshInstanceHeader.Position, staticMeshInstance->Position, 3 * sizeof(float));
				memcpy(staticMeshInstanceHeader.Rotation, staticMeshInstance->Rotation, 3 * sizeof(float));
				memcpy(staticMeshInstanceHeader.Scale, staticMeshInstance->Scale, 3 * sizeof(float));
				staticMeshInstanceHeader.HasLightExtension = staticMeshInstance->LightExt != nullptr;
				staticMeshInstanceHeader.LightExtensionIndex = staticMeshInstance->LightExt ? lightExtensionIndex : 0;

				if (staticMeshInstance->LightExt != nullptr)
				{
					LightExtension* lightExt = staticMeshInstance->LightExt.get();
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

			StaticMeshHeader staticMeshHeader = {};
			memcpy(staticMeshHeader.MeshName, staticMesh->MeshName.data(), staticMesh->MeshName.size());
			staticMeshHeader.InstanceCount = (uint32_t)staticMesh->Instances.size();
			staticMeshHeader.InstanceIndex = staticMeshInstanceIndex; // Place holder
			staticMeshHeaders.push_back(staticMeshHeader);
			stringItr += (uint32_t)(staticMesh->MeshName.size() + 1);
		}

		uint32_t meshHeadersOffset = 1024;
		uint32_t staticMeshInstancesOffset = meshHeadersOffset + (uint32_t)staticMeshHeaders.size() * sizeof(StaticMeshHeader);
		uint32_t lightExtensionsOffset = staticMeshInstancesOffset + (uint32_t)staticMeshInstanceHeaders.size() * sizeof(StaticMeshInstanceHeader);
		uint32_t stringsOffset = lightExtensionsOffset + (uint32_t)lightExtensionHeaders.size() * sizeof(LightExtensionHeader);
		
		tileHeader.MeshHeadersOffset = meshHeadersOffset;
		tileHeader.StaticMeshInstancesOffset = staticMeshInstancesOffset;
		tileHeader.LightExtensionsOffset = lightExtensionsOffset;

		tileHeader.NumLightExtensions = (uint32_t)lightExtensionHeaders.size();
		tileHeader.NumStaticMeshes = (uint32_t)staticMeshHeaders.size();
		tileHeader.NumStaticMeshInstances = (uint32_t)staticMeshInstanceHeaders.size();

		// Get mesh part headers
		int totalNumMeshParts = 0;
		std::vector<std::unique_ptr<FbxLoader>> fbxLoaders;
		for (size_t i = 0; i < tile->StaticMeshes.size(); i++)
		{
			StaticMeshStruct* staticMesh = tile->StaticMeshes[i].get();
			FbxLoader *loader = new FbxLoader(staticMesh);
			loader->Load(true, nullptr, nullptr);
			totalNumMeshParts += (int)loader->GetMeshPartHeaders().size();
			fbxLoaders.push_back(std::unique_ptr<FbxLoader>(loader));
		}

		uint32_t meshPartHeadersOffset = 1024 + (uint32_t)staticMeshHeaders.size() * sizeof(StaticMeshHeader) 
							+ (uint32_t)staticMeshInstanceHeaders.size() * sizeof(StaticMeshInstanceHeader)
							+ (uint32_t)lightExtensionHeaders.size() * sizeof(LightExtensionHeader);
		tileHeader.MeshPartsOffset = meshPartHeadersOffset;
		tileHeader.NumMeshParts = totalNumMeshParts;

		uint32_t currentMeshOffset = meshPartHeadersOffset + totalNumMeshParts * sizeof(MeshPartHeader);


		std::vector<MeshPartHeader> allMeshPartHeaders;
		uint32_t currentMeshPartHeaderIdx = 0;
		for (size_t i = 0; i < fbxLoaders.size(); i++)
		{
			FbxLoader* loader = fbxLoaders[i].get();
			loader->Load(false, &currentMeshOffset, &file);
			for (auto& meshPartHeader : loader->GetMeshPartHeaders())
			{
				currentMeshOffset += meshPartHeader.MeshDataSize;
				allMeshPartHeaders.push_back(meshPartHeader);
			}

			staticMeshHeaders[i].MeshPartIndex = currentMeshPartHeaderIdx;
			staticMeshHeaders[i].MeshPartCount = (uint32_t)loader->GetMeshPartHeaders().size();
			currentMeshPartHeaderIdx += staticMeshHeaders[i].MeshPartCount;
		}

		memcpy(headerBytes, &tileHeader, sizeof(TileHeader));
		file.seekp(0);
		file.write((char*)headerBytes, 1024);
		file.write((char*)staticMeshHeaders.data(), staticMeshHeaders.size() * sizeof(StaticMeshHeader));
		file.write((char*)staticMeshInstanceHeaders.data(), staticMeshInstanceHeaders.size() * sizeof(StaticMeshInstanceHeader));
		file.write((char*)lightExtensionHeaders.data(), lightExtensionHeaders.size() * sizeof(LightExtensionHeader));
		file.write((char*)allMeshPartHeaders.data(), allMeshPartHeaders.size() * sizeof(MeshPartHeader));

		file.close();
	}

	return true;
}