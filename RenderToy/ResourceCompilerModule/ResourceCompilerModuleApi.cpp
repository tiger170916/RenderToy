#include "ResourceCompilerModuleApi.h"
#include "ResourceCompiler.h"
#include "ResourceCompilerTile.h"
#include "ResourceLoaderImpl.h"

bool CompileResource(const char* rootFile)
{
	ResourceCompiler* resourceCompiler = new ResourceCompiler();

	return resourceCompiler->CompileRoot(rootFile);
}

ResourceCompilerModule::ResourceLoader* LoadFile(char* filePath)
{
	ResourceLoaderImpl* binLoader = new ResourceLoaderImpl();

	if (!binLoader->LoadFile(filePath))
	{
		delete binLoader;

		return nullptr;
	}

	return binLoader;
}