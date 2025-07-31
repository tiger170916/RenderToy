#include "SpotLight.h"

SpotLight::SpotLight(float effectiveRange, FVector3 position, FVector3 intensity, FRotator rotator, float aspectRatio, float fov)
	:LightExtension(effectiveRange, position, intensity), m_rotator(rotator), m_aspectRatio(aspectRatio), m_fov(fov)
{
	m_lightType = LightType::LightType_Spot;

	m_projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(fov * DirectX::XM_PI, aspectRatio, 0.1f, effectiveRange);
	XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYaw(rotator.Pitch, rotator.Yaw, rotator.Roll);

	XMVECTOR forwardDirXM = XMVector3Transform(XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f), rotation);
	XMVECTOR upDirXM = XMVector3Transform(XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), rotation);

	XMVector3Normalize(forwardDirXM);
	XMVector3Normalize(upDirXM);

	FVector3 forwardDirection = FVector3(forwardDirXM.m128_f32[0], forwardDirXM.m128_f32[1], forwardDirXM.m128_f32[2]);
	FVector3 upDirection = FVector3(upDirXM.m128_f32[0], upDirXM.m128_f32[1], upDirXM.m128_f32[2]);

	FVector3 focusPos = m_position + forwardDirection;

	m_transformMatrix = XMMatrixLookAtLH(
		XMVectorSet(m_position.X, m_position.Y, m_position.Z, 1.0f),
		XMVectorSet(focusPos.X, focusPos.Y, focusPos.Z, 1.0f),
		XMVectorSet(upDirection.X, upDirection.Y, upDirection.Z, 1.0f));
}

SpotLight::~SpotLight() {}