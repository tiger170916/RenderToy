#pragma once
#include <filesystem>
#include "ResourceStructs.h"

using namespace ResourceCompilerModule;

class BinaryFileLoader
{
public:
	bool LoadBinary(std::filesystem::path filePath);
};