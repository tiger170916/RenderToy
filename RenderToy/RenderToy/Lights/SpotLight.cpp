#include "SpotLight.h"

SpotLight::SpotLight(float effectiveRange, FVector3 position, FVector3 intensity, uint32_t uid, FRotator rotator, float aspectRatio, float fov)
	:LightExtension(effectiveRange, position, intensity, uid), m_rotator(rotator), m_aspectRatio(aspectRatio), m_fov(fov)
{
	m_lightType = LightType::LightType_Spot;

	m_projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(fov * DirectX::XM_PI, aspectRatio, 0.1f, effectiveRange);
}

SpotLight::~SpotLight() {}