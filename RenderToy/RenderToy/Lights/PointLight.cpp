#include "PointLight.h"

PointLight::PointLight(float effectiveRange, FVector3 position, FVector3 intensity)
	: LightExtension(effectiveRange, position, intensity)
{
	m_lightType = LightType::LightType_Point;
}

PointLight::~PointLight()
{

}