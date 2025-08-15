#pragma once

#include "../Includes.h"
#include "../Vectors.h"
#include "../GraphicsContext.h"
#include "../D3DResource.h"

enum class LightType
{
	LightType_Point = 0,
	LightType_Spot = 1,
};

class LightExtension
{
protected:
	LightType m_lightType;

	// Local position
	FVector3 m_position;

	float m_effectiveRange = 0;

	float m_nearPlane = 0.3f;

	uint32_t m_uid = 0;

	FVector3 m_intensity;

public:

	LightExtension(float effectiveRange, FVector3 position, FVector3 intensity, uint32_t uid);

	virtual ~LightExtension() = 0;

	inline LightType GetLightType() const { return m_lightType; }

	inline FVector3 GetPosition() const { return m_position; }

	inline FVector3 GetIntensity() const { return m_intensity; }

	inline uint32_t GetUid() const { return m_uid; }

	inline float GetEffectiveRange () const { return m_effectiveRange; }

	inline float GetNearPlane() const { return m_nearPlane; }

public:
	// Interfaces
	virtual bool Initialize(GraphicsContext* graphicsContext) = 0;

	virtual void DrawEffectiveFrustum(GraphicsContext* graphicsContext, ID3D12GraphicsCommandList* cmdList, FVector3 parentTransform) = 0;
};