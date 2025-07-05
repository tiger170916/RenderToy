#pragma once
#include "Includes.h"
#include "Camera.h"
#include "StaticMesh.h"

class World
{
private:
	std::unique_ptr<Camera> m_activeCamera; // TODO: switch between different cameras

	std::vector<std::shared_ptr<StaticMesh>> m_staticMeshes;

public:
	World();

	~World();

	void SpawnStaticMesh(std::shared_ptr<StaticMesh> staticMesh);

	void SpawnStaticMeshes(std::vector<std::shared_ptr<StaticMesh>>& staticMeshes);

	const std::vector<std::shared_ptr<StaticMesh>>& GetAllStaticMeshes() const { return m_staticMeshes; }
};