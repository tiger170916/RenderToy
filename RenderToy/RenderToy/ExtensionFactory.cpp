#include "ExtensionFactory.h"
#include "UidGenerator.h"

std::unique_ptr<LightExtension> ExtensionFactory::SpawnLightExtension(
	float offset[3],
	float rotation[3],
	float color[3],
	float intensity,
	float atteunationRadius,
	float aspectRatio,
	float fov)
{
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

	return lightExt;
}