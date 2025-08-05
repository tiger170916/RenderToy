#include "MeshFactory.h"

std::unique_ptr<MeshFactory> MeshFactory::_singleton = nullptr;

MeshFactory* MeshFactory::Get()
{
	if (!_singleton)
	{
		_singleton = std::unique_ptr<MeshFactory>(new MeshFactory());
	}

	return _singleton.get();
}

StaticMesh* MeshFactory::CreateStaticMesh()
{
	StaticMesh* newMesh = new StaticMesh(UidGenerator::Get()->GenerateUid());

	return newMesh;
}