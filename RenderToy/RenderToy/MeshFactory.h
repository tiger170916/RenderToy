#pragma once

#include "Includes.h"
#include "UidGenerator.h"
#include "StaticMesh.h"
#include "CriticalSection.h"

class MeshFactory
{
private:
	static std::unique_ptr<MeshFactory> _singleton;

	static std::unique_ptr<CriticalSection> _criticalSection;

	std::vector<std::unique_ptr<StaticMesh>> m_staticMeshes;

public:
	static MeshFactory* Get();

	StaticMesh* CreateStaticMesh();

	bool StaticMeshAddInstance(StaticMesh* mesh, const Transform& transform);

	~MeshFactory() = default;

private:
	MeshFactory();

};