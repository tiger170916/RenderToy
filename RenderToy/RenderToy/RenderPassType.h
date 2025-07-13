#pragma once
#include "Includes.h"

enum class RenderPass
{
	EARLY_Z_PASS = 1,
	GEOMETRY_PASS = 2,
	LIGHTING_PASS = 3,
};