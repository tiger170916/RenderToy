#pragma once
#include <filesystem>
#include <fstream>
#include "ResourceStructs.h"

using namespace ResourceCompilerModule;
/*
class BinaryFileLoader
{
public:
	bool LoadBinary(std::filesystem::path filePath);

	bool ReadHeader(std::fstream* file, ResourceCompilerModule::BinaryHeader& header);

	bool GetStaticMeshes(
		std::fstream* file,
		const ResourceCompilerModule::BinaryHeader& header,
		ResourceCompilerModule::StaticMeshHeader** ppStaticMeshHeaders,
		uint32_t& outNumMeshes);

	bool GetStaticMeshInstances(
		std::fstream* file,
		const ResourceCompilerModule::BinaryHeader& header,
		ResourceCompilerModule::StaticMeshInstanceHeader** ppStaticMeshInstanceHeaders,
		uint32_t& outNumInstances);

	bool GetLightExtensions(
		std::fstream* file,
		const ResourceCompilerModule::BinaryHeader& header,
		ResourceCompilerModule::LightExtensionHeader** ppLightExtensionHeaders,
		uint32_t& outNumLights);

	bool GetStaticMeshDefinitions(
		std::fstream* file,
		const ResourceCompilerModule::BinaryHeader& header,
		ResourceCompilerModule::StaticMeshDefinitionHeader** ppStaticMeshDefinitionHeaders,
		uint32_t& outNumStaticMeshDefinitions);

	bool GetMeshPartHeaders(
		std::fstream* file,
		const ResourceCompilerModule::BinaryHeader& header, 
		ResourceCompilerModule::MeshPartHeader** ppMeshPartHeaders,
		uint32_t& outNumMeshPartHeaders);

	bool GetMaterialHeaders(
		std::fstream* file,
		const ResourceCompilerModule::BinaryHeader& header,
		ResourceCompilerModule::MaterialHeader** ppMaterialHeaders,
		uint32_t& outNumMaterialHeaders);

	bool GetTextureHeaders(
		std::fstream* file,
		const ResourceCompilerModule::BinaryHeader& header,
		ResourceCompilerModule::TextureHeader** ppTextureHeaders,
		uint32_t& outNumTextureHeaders);
};*/