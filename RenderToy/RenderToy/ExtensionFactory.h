#pragma once

#include "Includes.h"
#include "lights/LightExtension.h"
#include "StaticMesh2.h"

class ExtensionFactory
{
public:
	static std::unique_ptr<LightExtension> SpawnLightExtension(
		float offset[3], 
		float rotation[3], 
		float color[3], 
		float intensity, 
		float atteunationRadius,
		float aspectRatio,
		float fov);
};