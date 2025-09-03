#include "ResourceCompilerModuleApi.h"
#include "ResourceCompiler.h"

bool CompileResource(const char* rootFile)
{
	ResourceCompiler* resourceCompiler = new ResourceCompiler();

	return resourceCompiler->CompileRoot(rootFile);
}