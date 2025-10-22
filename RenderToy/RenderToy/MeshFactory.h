#pragma once

#include "Includes.h"
#include "UidGenerator.h"
#include "StaticMesh.h"
#include "CriticalSection.h"
#include "Lights/LightExtension.h"

/// <summary>
/// MeshFactory class is responsible to create mesh and mesh contents.
/// </summary>
class MeshFactory
{
private:
	static std::unique_ptr<MeshFactory> _singleton;

	static std::unique_ptr<CriticalSection> _criticalSection;

public:
	static MeshFactory* Get();

	StaticMesh* CreateStaticMesh();

	bool StaticMeshAddInstance(StaticMesh* mesh, const Transform& transform);

	bool StaticMeshAddLightExtension(
		StaticMesh* mesh,
		uint32_t instanceIdx,
		LightType lightType,
		float offset[3],
		float rotation[3],
		float color[3],
		float intensity,
		float attenuationRadius,
		float aspectRatio,
		float fov);

	void StaticMeshAddPart(
		StaticMesh* mesh,
		uint32_t vertexOffset, 
		uint32_t vertexCount, 
		uint32_t bufferSize);

	~MeshFactory() = default;


private:
	MeshFactory();

};