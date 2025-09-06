#pragma once
#include <filesystem>
#include <fstream>
#include "ResourceStructs.h"

using namespace ResourceCompilerModule;

class BinaryFileLoader
{
public:
	bool LoadBinary(std::filesystem::path filePath);

	bool ReadHeader(std::fstream* file, ResourceCompilerModule::TileHeader& header);

	bool GetStaticMeshes(
		std::fstream* file,
		const ResourceCompilerModule::TileHeader& header,
		ResourceCompilerModule::StaticMeshHeader** ppStaticMeshHeaders,
		uint32_t& outNumMeshes);

	bool GetStaticMeshInstances(
		std::fstream* file,
		const ResourceCompilerModule::TileHeader& header,
		ResourceCompilerModule::StaticMeshInstanceHeader** ppStaticMeshInstanceHeaders,
		uint32_t& outNumInstances);

	bool GetLightExtensions(
		std::fstream* file,
		const ResourceCompilerModule::TileHeader& header,
		ResourceCompilerModule::LightExtensionHeader** ppLightExtensionHeaders,
		uint32_t& outNumLights);

	bool GetMeshParts(
		std::fstream* file,
		const ResourceCompilerModule::TileHeader& header,
		ResourceCompilerModule::MeshPartHeader** ppMeshPartHeaders,
		uint32_t& outMeshParts);
};