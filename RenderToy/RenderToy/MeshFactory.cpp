#include "MeshFactory.h"

std::unique_ptr<MeshFactory> MeshFactory::_singleton = nullptr;
std::unique_ptr<CriticalSection> MeshFactory::_criticalSection = std::unique_ptr<CriticalSection>(new CriticalSection());

MeshFactory::MeshFactory()
{
}

MeshFactory* MeshFactory::Get()
{
	if (!_singleton)
	{
		_criticalSection->EnterCriticalSection();

		if (!_singleton)
		{
			_singleton = std::unique_ptr<MeshFactory>(new MeshFactory());
		}

		_criticalSection->ExitCriticalSection();
	}

	return _singleton.get();
}

StaticMesh* MeshFactory::CreateStaticMesh()
{
	_criticalSection->EnterCriticalSection();

	StaticMesh* newMesh = new StaticMesh(UidGenerator::Get()->GenerateUid());
	m_staticMeshes.push_back(std::unique_ptr<StaticMesh>(newMesh));

	_criticalSection->ExitCriticalSection();

	return newMesh;
}


bool MeshFactory::StaticMeshAddInstance(StaticMesh* mesh, const Transform& transform)
{
	if (!mesh)
	{
		return false;
	}

	_criticalSection->EnterCriticalSection();

	uint32_t uid = UidGenerator::Get()->GenerateUid();
	mesh->AddInstance(transform, uid);

	_criticalSection->ExitCriticalSection();

	return true;
}