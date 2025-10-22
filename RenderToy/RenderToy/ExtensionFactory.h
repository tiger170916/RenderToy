#pragma once

#include "Includes.h"
#include "lights/LightExtension.h"
#include "StaticMesh2.h"

class ExtensionFactory
{
public:
	static bool SpawnLightExtension(
		StaticMesh2* staticMesh, 
		uint32_t instanceId, 
		float offset[3], 
		float rotation[3], 
		float color[3], 
		float intensity, 
		float atteunationRadius,
		float aspectRatio,
		float fov);
};