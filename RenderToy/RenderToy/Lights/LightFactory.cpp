#include "LightFactory.h"

std::unique_ptr<LightFactory> LightFactory::_singleton = nullptr;

LightFactory* LightFactory::Get()
{
	if (!_singleton)
	{
		_singleton = std::unique_ptr<LightFactory>(new LightFactory());
	}

	return _singleton.get();
}

LightFactory::~LightFactory()
{

}


LightFactory::LightFactory()
{

}

uint64_t LightFactory::SpawnPointLight(StaticMesh* parent, float effectiveRange, FVector3 position, FVector3 intensity)
{
	if (!parent)
	{
		return UINT64_MAX;
	}

	PointLight* light = new PointLight(effectiveRange, position, intensity);
	parent->AttachLightExtension(light);
	return ++m_lightUid;
}

uint64_t LightFactory::SpawnSpotLight(StaticMesh* parent, float effectiveRange, FVector3 position, FVector3 intensity, FRotator rotator, float aspectRatio, float fov)
{
	if (!parent)
	{
		return UINT64_MAX;
	}

	SpotLight* light = new SpotLight(effectiveRange, position, intensity, rotator, aspectRatio, fov);
	parent->AttachLightExtension(light);
	return ++m_lightUid;
}