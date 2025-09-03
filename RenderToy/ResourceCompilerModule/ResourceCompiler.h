#pragma once
#include "fbxsdk.h"
#include "3rdParty/json.hpp"
#include <vector>
#include <set>
#include <filesystem>
#include "World.h"

using json = nlohmann::json;

class ResourceCompiler
{
public:
	ResourceCompiler();

	~ResourceCompiler();

	bool CompileRoot(const char* rootFile);

private:
	bool ParseTileList(json obj, World* world);
};