#pragma once

#include <set>
#include <vector>
#include <filesystem>
#include <fstream>
#include "3rdParty/json.hpp"
#include "Structs.h"
#include "ResourceStructs.h"

using json = nlohmann::json;
using namespace ResourceCompilerModule;


class Textures
{
private:
	std::vector<TextureDefine> m_textures;

public:
	void AddTexture(
		std::string name,
		std::string file);

	bool PackToBinary(std::filesystem::path rootFilePath);
};