#include "LightExtension.h"
#include "../GraphicsUtils.h"

LightExtension::LightExtension(
	uint32_t uid,
	float offset[3],
	float rotation[3],
	float color[3],
	float intensity,
	float atteunationRadius,
	float aspectRatio,
	float fov) 
	: ObjectExtension(uid),
	m_offset(FVector3(offset[0], offset[1], offset[2])),
	m_rotation(FVector3(rotation[0], rotation[1], rotation[2])),
	m_color(FVector3(color[0], color[1], color[2])),
	m_intensity(intensity),
	m_attenuationRadius(atteunationRadius),
	m_aspectRatio(aspectRatio),
	m_fov(fov)
{
	m_projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(fov * DirectX::XM_PI, aspectRatio, m_nearPlane, m_attenuationRadius);
}

void LightExtension::UpdateLightConstants(
	LightConstants& lightConsts,
	const FVector3& parentPos,
	const UINT& lightAtlasOffsetX,
	const UINT& lightAtlasOffsetY, 
	const UINT& lightMapSize,
	const UINT& parentUid)
{
	lightConsts.LightUid = m_uid;
	lightConsts.LightType = static_cast<UINT>(m_lightType);
	lightConsts.FarPlane = m_attenuationRadius;
	lightConsts.NearPlane = m_nearPlane;
	lightConsts.Intensity[0] = m_color.X * m_intensity;
	lightConsts.Intensity[1] = m_color.Y * m_intensity;
	lightConsts.Intensity[2] = m_color.Z * m_intensity;
	lightConsts.Position[0] = parentPos.X + m_offset.X;
	lightConsts.Position[1] = parentPos.Y + m_offset.Y;
	lightConsts.Position[2] = parentPos.Z + m_offset.Z;
	lightConsts.Position[3] = 1.0f;
	lightConsts.ShadowBufferOffsetX = lightAtlasOffsetX;
	lightConsts.ShadowBufferOffsetY = lightAtlasOffsetY;
	lightConsts.ShadowBufferSize = lightMapSize;
	lightConsts.LightParentUid = parentUid;
	
	FVector3 forwardDir;
	XMMATRIX viewMatrix = GraphicsUtils::ViewMatrixFromPositionRotation(FVector3(lightConsts.Position[0], lightConsts.Position[1], lightConsts.Position[2]), FRotator(m_rotation.X, m_rotation.Y, m_rotation.Z), forwardDir);

	DirectX::XMStoreFloat4x4(&lightConsts.Transform, DirectX::XMMatrixTranspose(m_projectionMatrix) * DirectX::XMMatrixTranspose(viewMatrix));
	DirectX::XMStoreFloat4x4(&lightConsts.ViewMatrix, DirectX::XMMatrixTranspose(viewMatrix));
}