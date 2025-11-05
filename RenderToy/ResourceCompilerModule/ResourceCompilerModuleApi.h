#pragma once

#pragma once

#include "Windows.h"

#include "ResourceCompilerTile.h"
#include "ResourceLoader.h"
#include <fstream>

#ifdef RESOURCE_COMPILER_MODULE_EXPORT
#define RESOURCE_COMPILER_MODULE_API __declspec(dllexport)
#else
#define RESOURCE_COMPILER_MODULE_API __declspec(dllimport)
#endif

extern "C"
{
	/// <summary>
	/// Compile the resource
	/// </summary>
	RESOURCE_COMPILER_MODULE_API bool CompileResource(const char* rootFile);

	/// <summary>
	/// Load a binary file
	/// </summary>
	RESOURCE_COMPILER_MODULE_API ResourceCompilerModule::ResourceLoader* LoadFile(char* filePath);
}