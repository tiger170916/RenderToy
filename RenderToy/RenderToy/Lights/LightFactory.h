#pragma once

#include "../Includes.h"
#include "../StaticMesh.h"
#include "PointLight.h"
#include "SpotLight.h"

class LightFactory
{
private:
	static std::unique_ptr<LightFactory> _singleton;

public:
	static LightFactory* Get();

	/// <summary>
	/// Create a point light, and attach it to a static mesh
	/// </summary>
	void SpawnPointLight(StaticMesh* parent, const uint32_t& parentInstanceIdx, float effectiveRange, FVector3 position, FVector3 intensity);

	/// <summary>
	/// Create a spot light, and attach it to a static mesh.
	/// </summary>
	void SpawnSpotLight(StaticMesh* parent, const uint32_t& parentInstanceIdx, float effectiveRange, FVector3 position, FVector3 intensity, FRotator rotator, float aspectRatio, float fov);

	~LightFactory();

private:
	LightFactory();
};