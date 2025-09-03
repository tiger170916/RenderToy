#pragma once

#pragma once

#include "Windows.h"

#ifdef RESOURCE_COMPILER_MODULE_EXPORT
#define RESOURCE_COMPILER_MODULE_API __declspec(dllexport)
#else
#define RESOURCE_COMPILER_MODULE_API __declspec(dllimport)
#endif

extern "C"
{
	RESOURCE_COMPILER_MODULE_API bool CompileResource(const char* rootFile);
}