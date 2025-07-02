#pragma once

#include "Windows.h"

#ifdef RENDER_TOY_EXPORT
#define RENDER_TOY_API __declspec(dllexport)
#else
#define RENDER_TOY_API __declspec(dllimport)
#endif

extern "C"
{
	RENDER_TOY_API bool InitializeRenderer();
}