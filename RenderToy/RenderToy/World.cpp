#include "World.h"

World::World() {}

World::~World() {}

void World::SpawnStaticMesh(std::shared_ptr<StaticMesh> staticMesh)
{
	m_staticMeshes.push_back(staticMesh);
}

void World::SpawnStaticMeshes(std::vector<std::shared_ptr<StaticMesh>>& staticMeshes)
{
	for (auto mesh : staticMeshes)
	{
		m_staticMeshes.push_back(mesh);
	}
}