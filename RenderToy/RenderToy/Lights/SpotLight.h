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

	XMMATRIX m_transformMatrix;

protected:
	SpotLight(float effectiveRange, FVector3 position, FVector3 intensity, FRotator rotator, float aspectRatio, float fov);

public:
	~SpotLight();

	const XMMATRIX GetProjectionMatrix() const { return m_projectionMatrix; }

	const XMMATRIX GetTransformMatrix() const { return m_transformMatrix; }
};