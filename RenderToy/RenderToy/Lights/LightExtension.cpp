#include "LightExtension.h"

LightExtension::LightExtension(float effectiveRange, FVector3 position, FVector3 intensity, uint32_t uid)
	: m_effectiveRange(effectiveRange), m_position(position), m_intensity(intensity), m_uid(uid)
{

}

LightExtension::~LightExtension()
{

}