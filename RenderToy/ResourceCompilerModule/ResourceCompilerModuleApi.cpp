#include "ResourceCompilerModuleApi.h"
#include "ResourceCompiler.h"
#include "ResourceCompilerTile.h"
#include "ResourceLoaderImpl.h"

bool ResourceCompilerModule::CompileResource(const char* rootFile)
{
	ResourceCompiler* resourceCompiler = new ResourceCompiler();

	return resourceCompiler->CompileRoot(rootFile);
}

ResourceCompilerModule::ResourceLoader* ResourceCompilerModule::LoadFile(const char* filePath)
{
	ResourceLoaderImpl* binLoader = new ResourceLoaderImpl();

	if (!binLoader->LoadFile(filePath))
	{
		delete binLoader;

		return nullptr;
	}

	return binLoader;
}

ResourceCompilerModule::ResourceType ResourceCompilerModule::GetResourceType(const char* filePath)
{
	std::shared_ptr<ResourceLoaderImpl> binLoader = std::make_shared<ResourceLoaderImpl>();

	return binLoader->ReadResourceType(filePath);
}