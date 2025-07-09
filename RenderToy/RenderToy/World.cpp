#include "World.h"

World::World() {}

World::~World() {}

bool World::Initialize(ID3D12Device* pDevice, DescriptorHeapManager* descriptorHeapManager)
{
	if (m_initialized)
	{
		return true;
	}

	if (!pDevice || !descriptorHeapManager)
	{
		return false;
	}

	m_uniformFrameConstantBuffer = std::unique_ptr<ConstantBuffer<UniformFrameConstants>>(new ConstantBuffer<UniformFrameConstants>());
	if (!m_uniformFrameConstantBuffer->Initialize(pDevice, descriptorHeapManager))
	{
		return false;
	}

	m_initialized = true;
	return true;
}

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