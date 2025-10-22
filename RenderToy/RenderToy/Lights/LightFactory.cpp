#include "LightFactory.h"
#include "../UidGenerator.h"

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

void LightFactory::SpawnPointLight(GraphicsContext* graphicsContext, StaticMesh* parent, const uint32_t& parentInstanceIdx, float effectiveRange, FVector3 position, FVector3 intensity)
{
	if (!parent)
	{
		return;
	}
	/*
	PointLight* light = new PointLight(effectiveRange, position, intensity, UidGenerator::Get()->GenerateUid());
	parent->AttachLightExtension(light, parentInstanceIdx);*/
}

void LightFactory::SpawnSpotLight(GraphicsContext* graphicsContext, StaticMesh* parent, const uint32_t& parentInstanceIdx, float effectiveRange, FVector3 position, FVector3 intensity, FRotator rotator, float aspectRatio, float fov)
{
	/*
	if (!parent)
	{
		return;
	}

	SpotLight* light = new SpotLight(effectiveRange, position, intensity, UidGenerator::Get()->GenerateUid(), rotator, aspectRatio, fov);
	light->Initialize(graphicsContext);
	parent->AttachLightExtension(light, parentInstanceIdx);*/
}