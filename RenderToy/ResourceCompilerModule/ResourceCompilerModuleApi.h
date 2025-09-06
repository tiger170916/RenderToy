#pragma once

#pragma once

#include "Windows.h"
#include "ResourceStructs.h"
#include <fstream>

#ifdef RESOURCE_COMPILER_MODULE_EXPORT
#define RESOURCE_COMPILER_MODULE_API __declspec(dllexport)
#else
#define RESOURCE_COMPILER_MODULE_API __declspec(dllimport)
#endif

extern "C"
{
	RESOURCE_COMPILER_MODULE_API bool CompileResource(const char* rootFile);

	RESOURCE_COMPILER_MODULE_API bool ReadHeader(std::fstream* file, ResourceCompilerModule::TileHeader& header);

	RESOURCE_COMPILER_MODULE_API bool GetStaticMeshes(
		std::fstream* file, 
		const ResourceCompilerModule::TileHeader& header, 
		ResourceCompilerModule::StaticMeshHeader** ppStaticMeshHeaders,
		uint32_t& outNumMeshes);

	RESOURCE_COMPILER_MODULE_API bool GetStaticMeshInstances(
		std::fstream* file,
		const ResourceCompilerModule::TileHeader& header,
		ResourceCompilerModule::StaticMeshInstanceHeader** ppStaticMeshHeaders,
		uint32_t& outNumInstances);

	RESOURCE_COMPILER_MODULE_API bool GetLightExtensions(
		std::fstream* file,
		const ResourceCompilerModule::TileHeader& header,
		ResourceCompilerModule::LightExtensionHeader** ppLightExtensionHeaders,
		uint32_t& outNumLights);

	RESOURCE_COMPILER_MODULE_API bool GetMeshParts(
		std::fstream* file,
		const ResourceCompilerModule::TileHeader& header,
		ResourceCompilerModule::MeshPartHeader** ppMeshPartHeaders,
		uint32_t& outMeshParts);
}