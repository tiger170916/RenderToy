#pragma once

#include "../Includes.h"
#include "../Vectors.h"

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

	FVector3 m_intensity;

public:
	LightExtension(float effectiveRange, FVector3 position, FVector3 intensity);

	virtual ~LightExtension() = 0;

	inline LightType GetLightType() const { return m_lightType; }

	inline FVector3 GetPosition() const { return m_position; }

	inline FVector3 GetIntensity() const { return m_intensity; }
};