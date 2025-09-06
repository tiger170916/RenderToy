#include "ResourceCompilerModuleApi.h"
#include "ResourceCompiler.h"
#include "BinaryFileLoader.h"

bool CompileResource(const char* rootFile)
{
	ResourceCompiler* resourceCompiler = new ResourceCompiler();

	return resourceCompiler->CompileRoot(rootFile);
}

bool ReadHeader(std::fstream* file, ResourceCompilerModule::TileHeader& header)
{
	BinaryFileLoader loader;
	return loader.ReadHeader(file, header);
}

bool GetStaticMeshes(
	std::fstream* file,
	const ResourceCompilerModule::TileHeader& header,
	ResourceCompilerModule::StaticMeshHeader** ppStaticMeshHeaders,
	uint32_t& outNumMeshes)
{
	BinaryFileLoader loader;
	return loader.GetStaticMeshes(file, header, ppStaticMeshHeaders, outNumMeshes);
}

bool GetStaticMeshInstances(
	std::fstream* file,
	const ResourceCompilerModule::TileHeader& header,
	ResourceCompilerModule::StaticMeshInstanceHeader** ppStaticMeshInstanceHeaders,
	uint32_t& outNumInstances)
{
	BinaryFileLoader loader;
	return loader.GetStaticMeshInstances(file, header, ppStaticMeshInstanceHeaders, outNumInstances);
}

bool GetLightExtensions(
	std::fstream* file,
	const ResourceCompilerModule::TileHeader& header,
	ResourceCompilerModule::LightExtensionHeader** ppLightExtensionHeaders,
	uint32_t& outNumLights)
{
	BinaryFileLoader loader;
	return loader.GetLightExtensions(file, header, ppLightExtensionHeaders, outNumLights);
}

bool GetMeshParts(
	std::fstream* file,
	const ResourceCompilerModule::TileHeader& header,
	ResourceCompilerModule::MeshPartHeader** ppMeshPartHeaders,
	uint32_t& outMeshParts)
{
	BinaryFileLoader loader;
	return loader.GetMeshParts(file, header, ppMeshPartHeaders, outMeshParts);
}