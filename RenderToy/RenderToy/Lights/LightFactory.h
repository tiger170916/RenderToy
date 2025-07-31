#pragma once

#include "../Includes.h"
#include "../StaticMesh.h"
#include "PointLight.h"
#include "SpotLight.h"

class LightFactory
{
private:
	static std::unique_ptr<LightFactory> _singleton;

	uint64_t m_lightUid = 0;

public:
	static LightFactory* Get();

	/// <summary>
	/// Create a point light, and attach it to a static mesh
	/// </summary>
	uint64_t SpawnPointLight(StaticMesh* parent, float effectiveRange, FVector3 position, FVector3 intensity);

	/// <summary>
	/// Create a spot light, and attach it to a static mesh.
	/// </summary>
	uint64_t SpawnSpotLight(StaticMesh* parent, float effectiveRange, FVector3 position, FVector3 intensity, FRotator rotator, float aspectRatio, float fov);

	~LightFactory();

private:
	LightFactory();
};