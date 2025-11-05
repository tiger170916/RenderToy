#pragma once

#include <set>
#include <vector>
#include <filesystem>
#include <fstream>
#include "3rdParty/json.hpp"
#include "InternalStructs.h"
#include "ResourceHeaders.h"

using json = nlohmann::json;

class Textures
{
private:
	std::vector<TextureDefineInternal> m_textures;

public:
	void AddTexture(
		std::string name,
		std::string file);

	bool PackToBinary(std::filesystem::path rootFilePath);
};