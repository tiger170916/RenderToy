#include "ResourceCompiler.h"
#include <fstream>
#include <memory>

ResourceCompiler::ResourceCompiler() {}

ResourceCompiler::~ResourceCompiler() {}

bool ResourceCompiler::CompileRoot(const char* rootFile)
{
	if (rootFile == nullptr)
	{
		return false;
	}

	if (!std::filesystem::exists(rootFile))
	{
		return false;
	}

	std::filesystem::path rootFilePath = std::filesystem::absolute(rootFile);
	std::filesystem::path rootPath = std::filesystem::path(rootFile).parent_path();
	std::filesystem::path parentDir = rootPath.parent_path();
	

	std::unique_ptr<World> world = std::unique_ptr<World>(new World());

	std::ifstream file(rootFile);
	bool succ = true;
	try
	{
		json data = json::parse(file);
		if (data.contains("WorldRoot") && data["WorldRoot"].is_object())
		{
			auto worldRoot = data["WorldRoot"];
			// Get tile list
			if (worldRoot.contains("Tiles") && worldRoot["Tiles"].is_array())
			{
				auto tiles = worldRoot["Tiles"];
				if (!ParseTileList(tiles, world.get()))
				{
					return false;
				}
				
			}

			// Get contents of each tile.
			for (const auto& tile : world->GetTiles())
			{
				if (worldRoot.contains(tile->Name))
				{
					world->AddTileContent(worldRoot[tile->Name], tile->Name, rootPath);
				}
			}
		}
	}
	catch (std::exception)
	{
		succ = false;
		return succ;
	}

	succ = world->PackToBinary(rootFile);

	return succ;
}

bool ResourceCompiler::ParseTileList(json obj, World* world)
{
	for (auto& tile : obj)
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

	return true;
}