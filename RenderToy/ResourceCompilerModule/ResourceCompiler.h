#pragma once
#include "fbxsdk.h"
#include "3rdParty/json.hpp"
#include <vector>
#include <set>
#include <filesystem>
#include "World.h"
#include "Meshes.h"
#include "Textures.h"

using json = nlohmann::json;

class ResourceCompiler
{
public:
	ResourceCompiler();

	~ResourceCompiler();

	bool CompileRoot(const char* rootFile);

private:
	bool ParseFile(std::ifstream* file, std::filesystem::path filePath);

	bool ParseWorldRoot(const json& data, std::filesystem::path filePath);

	bool ParseMeshes(const json& data, std::filesystem::path filePath);

	bool ParseTextures(const json& data, std::filesystem::path filePath);
};