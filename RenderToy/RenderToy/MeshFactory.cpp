#include "MeshFactory.h"
#include "Lights/SpotLight.h"

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

bool MeshFactory::StaticMeshAddLightExtension(
	StaticMesh* mesh,
	uint32_t instanceIdx,
	LightType lightType,
	float offset[3],
	float rotation[3],
	float color[3],
	float intensity,
	float attenuationRadius,
	float aspectRatio,
	float fov)
{
	/*
	if (!mesh) 
	{
		return false;
	}

	if (instanceIdx >= mesh->GetNumInstances())
	{
		return false;
	}

	uint32_t uid = UidGenerator::Get()->GenerateUid();
	switch (lightType)
	{
	case LightType::LightType_Spot:
	{
		//float effectiveRange, FVector3 position, FVector3 intensity, uint32_t uid, FRotator rotator, float aspectRatio, float fov
		SpotLight* spotLight = new SpotLight(
			attenuationRadius,
			FVector3(offset[0], offset[1], offset[2]), 
			FVector3(color[0], color[1], color[2]) * intensity,
			uid,
			FRotator(rotation[0], rotation[1], rotation[2]),
			aspectRatio,
			fov);

		if (!mesh->StaticMeshAddLightExtension(instanceIdx, spotLight, uid))
		{
			delete spotLight;
		}

		break;
	}
	}*/

	return true;
}

void MeshFactory::StaticMeshAddPart(
	StaticMesh* mesh,
	uint32_t vertexOffset,
	uint32_t vertexCount,
	uint32_t bufferSize) 
{
	if (!mesh)
	{
		return;
	}

	mesh->AddMeshPart(vertexOffset, vertexCount, bufferSize);
}