#pragma once

#include "../Includes.h"
#include "LightExtension.h"

class SpotLight : public LightExtension
{
friend class LightFactory;

private:
	FRotator m_rotator;

	float m_aspectRatio;

	float m_fov;

	XMMATRIX m_projectionMatrix;

protected:
	SpotLight(float effectiveRange, FVector3 position, FVector3 intensity, uint32_t uid, FRotator rotator, float aspectRatio, float fov);

public:
	~SpotLight();

	const XMMATRIX GetProjectionMatrix() const { return m_projectionMatrix; }

	const FRotator GetRotator() const { return m_rotator; }
};