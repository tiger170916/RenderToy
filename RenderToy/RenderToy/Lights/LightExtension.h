#pragma once
#include "../ObjectExtension.h"
#include "../Includes.h"
#include "../Vectors.h"
#include "../GraphicsContext.h"
#include "../D3DResource.h"
#include "../LightStructs.h"

enum class LightType
{
	LightType_Point = 0,
	LightType_Spot = 1,
	LightType_Area = 2,
};

class LightExtension : public ObjectExtension
{
friend class ExtensionFactory;

private:
	LightType m_lightType = LightType::LightType_Spot;

	// Local position
	FVector3 m_offset;
	FVector3 m_rotation;
	FVector3 m_color;
	float m_intensity;
	float m_attenuationRadius;
	float m_aspectRatio;
	float m_fov;

	float m_nearPlane = 0.3f;

	XMMATRIX m_projectionMatrix;
	XMMATRIX m_viewMatrix;
	XMMATRIX m_worldMatrix;
	XMMATRIX m_transformMatrix;

protected:

	LightExtension(
		uint32_t uid,
		float offset[3],
		float rotation[3],
		float color[3],
		float intensity,
		float atteunationRadius,
		float aspectRatio,
		float fov);

public:
	FVector3 GetOffset() const { return m_offset; }

	FVector3 GetIntensity() const { return m_color * m_intensity; }

	void UpdateLightConstants(
		LightConstants& lightConsts,
		const FVector3& parentPos,
		const FRotator& parentRotation,
		const UINT& lightAtlasOffsetX, 
		const UINT& lightAtlasOffsetY, 
		const UINT& lightMapSize,
		const UINT& lightUid);

	~LightExtension() = default;
};