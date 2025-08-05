#include "PointLight.h"

PointLight::PointLight(float effectiveRange, FVector3 position, FVector3 intensity, uint32_t uid)
	: LightExtension(effectiveRange, position, intensity, uid)
{
	m_lightType = LightType::LightType_Point;
}

PointLight::~PointLight()
{

}