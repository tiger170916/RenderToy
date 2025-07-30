#pragma once

#include "../Includes.h"

enum class LightType
{
	LightType_Point = 0,
	LightType_Spot = 1,
};

class LightExtension
{
protected:
	LightType m_lightType;

public:
	LightExtension();

	virtual ~LightExtension() = 0;

	inline LightType GetLightType() const { return m_lightType; }
};