#include "PointLight.h"

PointLight::PointLight(FVector3 localPosition, FVector3 radiance)
	:m_localPosition(localPosition), m_radiance(radiance)
{
	m_lightType = LightType::LightType_Point;
}

PointLight::~PointLight()
{

}