#pragma once

#include "Includes.h"
#include "UidGenerator.h"
#include "StaticMesh.h"

class MeshFactory
{
private:
	static std::unique_ptr<MeshFactory> _singleton;

public:
	static MeshFactory* Get();

	StaticMesh* CreateStaticMesh();

	~MeshFactory() = default;

private:
	MeshFactory() = default;

};