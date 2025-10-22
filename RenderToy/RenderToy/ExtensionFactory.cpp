#include "ExtensionFactory.h"
#include "UidGenerator.h"

bool ExtensionFactory::SpawnLightExtension(
	StaticMesh2* staticMesh,
	uint32_t instanceId,
	float offset[3],
	float rotation[3],
	float color[3],
	float intensity,
	float atteunationRadius,
	float aspectRatio,
	float fov)
{
	if (!staticMesh)
	{
		return false;
	}

	uint32_t uid = UidGenerator::Get()->GenerateUid();
	LightExtension* lightExtPtr = new LightExtension(
		uid,
		offset,
		rotation,
		color,
		intensity,
		atteunationRadius,
		aspectRatio,
		fov);

	std::unique_ptr<LightExtension> lightExt = std::unique_ptr<LightExtension>(lightExtPtr);

	return staticMesh->AttachLightExtension(instanceId, lightExt);
}