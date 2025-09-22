#include "ResourceCompiler.h"
#include <fstream>
#include <memory>

ResourceCompiler::ResourceCompiler() {}

ResourceCompiler::~ResourceCompiler() {}

bool ResourceCompiler::CompileRoot(const char* filePathStr)
{
	if (filePathStr == nullptr)
	{
		return false;
	}

	if (!std::filesystem::exists(filePathStr))
	{
		return false;
	}

	std::filesystem::path filePath = std::filesystem::absolute(filePathStr);

	std::ifstream file(filePath);
	if (!file.is_open())
	{
		return false;
	}

	
	bool succ = ParseFile(&file, filePath);

	file.close();
	return succ;
}

bool ResourceCompiler::ParseFile(std::ifstream* file, std::filesystem::path filePath)
{
	if (!file)
	{
		return false;
	}

	try
	{
		json data = json::parse(*file);

		if (!data.contains("Type") || !data["Type"].is_string())
		{
			return false;
		}

		if (data["Type"] == "World")
		{
			return ParseWorldRoot(data, filePath);
		}
		else if (data["Type"] == "Meshes")
		{
			return ParseMeshes(data, filePath);
		}
		else if (data["Type"] == "Textures")
		{
			return ParseTextures(data, filePath);
		}
	}
	catch (std::exception)
	{
		return false;
	}

	return false;
}

bool ResourceCompiler::ParseTextures(const json& data, std::filesystem::path filePath)
{
	if (data.contains("Textures") && data["Textures"].is_array())
	{
		std::unique_ptr<Textures> textures = std::unique_ptr<Textures>(new Textures());
		for (auto& meshItem : data["Textures"])
		{
			if (meshItem.contains("Name") && meshItem["Name"].is_string() &&
				meshItem.contains("Path") && meshItem["Path"].is_string())
			{
				std::string texName = meshItem["Name"];
				std::string texPath = meshItem["Path"];

				textures->AddTexture(texName, texPath);
			}
		}

		return textures->PackToBinary(filePath);
	}

	return false;
}

bool ResourceCompiler::ParseMeshes(const json& data, std::filesystem::path filePath)
{
	if (data.contains("Meshes") && data["Meshes"].is_array())
	{
		std::unique_ptr<Meshes> meshes = std::unique_ptr<Meshes>(new Meshes());

		for (auto& meshItem : data["Meshes"])
		{
			if (meshItem.contains("Name") && meshItem["Name"].is_string() &&
				meshItem.contains("MeshAsset") && meshItem["MeshAsset"].is_string())
			{
				std::string overrideMaterialName;
				if (meshItem.contains("MaterialOverride") && meshItem["MaterialOverride"].is_string())
				{
					overrideMaterialName = meshItem["MaterialOverride"];
				}

				meshes->AddStaticMesh(meshItem["Name"], meshItem["MeshAsset"], overrideMaterialName);
			}
		}

		// Parse material definitions
		if (data.contains("Materials") && data["Materials"].is_array())
		{
			for (auto& matItem : data["Materials"])
			{
				if (!matItem.contains("Name"))
				{
					continue;
				}

				std::string baseColorTexName;
				std::string baseColorTexFile;
				std::string metallicTexName;
				std::string metallicTexFile;
				std::string	roughnessTexName;
				std::string roughnessTexFile;
				std::string normalTexName;
				std::string normalTexFile;

				if (matItem.contains("BaseColorTexture"))
				{
					baseColorTexName = matItem["BaseColorTexture"]["Name"];
					baseColorTexFile = matItem["BaseColorTexture"]["File"];
				}

				if (matItem.contains("MetallicTexture"))
				{
					metallicTexName = matItem["MetallicTexture"]["Name"];
					metallicTexFile = matItem["MetallicTexture"]["File"];
				}

				if (matItem.contains("RoughnessTexture"))
				{
					roughnessTexName = matItem["RoughnessTexture"]["Name"];
					roughnessTexFile = matItem["RoughnessTexture"]["File"];
				}

				if (matItem.contains("NormalTexture"))
				{
					normalTexName = matItem["NormalTexture"]["Name"];
					normalTexFile = matItem["NormalTexture"]["File"];
				}

				meshes->AddMaterial(
					matItem["Name"],
					baseColorTexName,
					baseColorTexFile,
					metallicTexName,
					metallicTexFile,
					roughnessTexName,
					roughnessTexFile,
					normalTexName,
					normalTexFile);
			}
		}

		return meshes->PackToBinary(filePath);
	}

	return false;
}


bool ResourceCompiler::ParseWorldRoot(const json& data, std::filesystem::path filePath)
{
	if (data.contains("WorldRoot") && data["WorldRoot"].is_object())
	{
		std::unique_ptr<World> world = std::unique_ptr<World>(new World());

		auto worldRoot = data["WorldRoot"];
		// Get tile list
		if (worldRoot.contains("Tiles") && worldRoot["Tiles"].is_array())
		{
			auto tiles = worldRoot["Tiles"];
			for (auto& tile : tiles)
			{
				if (!tile.contains("Name") || !tile.contains("BboxMin") || !tile.contains("BboxMax"))
				{
					return false;
				}

				std::string tileName = tile["Name"];
				auto bboxMin = tile["BboxMin"];
				auto bboxMax = tile["BboxMax"];
				const float bboxMinFloat[2] = { bboxMin[0], bboxMin[1] };
				const float bboxMaxFloat[2] = { bboxMax[0], bboxMax[1] };

				world->AddTile(tileName, bboxMinFloat, bboxMaxFloat);
			}

		}

		// Get contents of each tile.
		for (const auto& tile : world->GetTiles())
		{
			if (worldRoot.contains(tile->Name))
			{
				world->AddTileContent(worldRoot[tile->Name], tile->Name, filePath);
			}
		}

		return world->PackToBinary(filePath);
	}

	return false;
}
