#include "ResourceCompilerModuleApi.h"
#include "ResourceCompiler.h"
#include "BinaryFileLoader.h"

bool CompileResource(const char* rootFile)
{
	ResourceCompiler* resourceCompiler = new ResourceCompiler();

	return resourceCompiler->CompileRoot(rootFile);
}

bool ReadHeader(std::fstream* file, ResourceCompilerModule::BinaryHeader& header)
{
	BinaryFileLoader loader;
	return loader.ReadHeader(file, header);
}

bool GetStaticMeshes(
	std::fstream* file,
	const ResourceCompilerModule::BinaryHeader& header,
	ResourceCompilerModule::StaticMeshHeader** ppStaticMeshHeaders,
	uint32_t& outNumMeshes)
{
	BinaryFileLoader loader;
	return loader.GetStaticMeshes(file, header, ppStaticMeshHeaders, outNumMeshes);
}

bool GetStaticMeshInstances(
	std::fstream* file,
	const ResourceCompilerModule::BinaryHeader& header,
	ResourceCompilerModule::StaticMeshInstanceHeader** ppStaticMeshInstanceHeaders,
	uint32_t& outNumInstances)
{
	BinaryFileLoader loader;
	return loader.GetStaticMeshInstances(file, header, ppStaticMeshInstanceHeaders, outNumInstances);
}

bool GetLightExtensions(
	std::fstream* file,
	const ResourceCompilerModule::BinaryHeader& header,
	ResourceCompilerModule::LightExtensionHeader** ppLightExtensionHeaders,
	uint32_t& outNumLights)
{
	BinaryFileLoader loader;
	return loader.GetLightExtensions(file, header, ppLightExtensionHeaders, outNumLights);
}

bool GetStaticMeshDefinitions(
	std::fstream* file,
	const ResourceCompilerModule::BinaryHeader& header,
	ResourceCompilerModule::StaticMeshDefinitionHeader** ppStaticMeshDefinitionHeaders,
	uint32_t& outNumStaticMeshDefinitions)
{
	BinaryFileLoader loader;
	return loader.GetStaticMeshDefinitions(file, header, ppStaticMeshDefinitionHeaders, outNumStaticMeshDefinitions);
}

bool GetMeshPartHeaders(
	std::fstream* file,
	const ResourceCompilerModule::BinaryHeader& header,
	ResourceCompilerModule::MeshPartHeader** ppMeshPartHeaders,
	uint32_t& outNumMeshPartHeaders)
{
	BinaryFileLoader loader;
	return loader.GetMeshPartHeaders(file, header, ppMeshPartHeaders, outNumMeshPartHeaders);
}

bool GetMaterialHeaders(
	std::fstream* file,
	const ResourceCompilerModule::BinaryHeader& header,
	ResourceCompilerModule::MaterialHeader** ppMaterialHeaders,
	uint32_t& outNumMaterialHeaders)
{
	BinaryFileLoader loader;
	return loader.GetMaterialHeaders(file, header, ppMaterialHeaders, outNumMaterialHeaders);
}

bool GetTextureHeaders(
	std::fstream* file,
	const ResourceCompilerModule::BinaryHeader& header,
	ResourceCompilerModule::TextureHeader** ppTextureHeaders,
	uint32_t& outNumTextureHeaders)
{
	BinaryFileLoader loader;
	return loader.GetTextureHeaders(file, header, ppTextureHeaders, outNumTextureHeaders);
}