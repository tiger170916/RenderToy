#pragma once

#include "Includes.h"
#include "Vectors.h"
#include "SceneObjectComponent.h"

class Camera : public SceneObjectComponent
{
protected:
	UINT m_width = 0;

	UINT m_height = 0;

	XMMATRIX m_projectionMatrix = XMMatrixIdentity();

	XMMATRIX m_viewMatrix = XMMatrixIdentity();

	float m_pixelStepScale = 0.0f;

	// Basis of coordinate
	const FVector3 m_basisForward = FVector3(0.0f, 0.0f, 1.0f);

	const FVector3 m_basisRight = FVector3(1.0f, 0.0f, 0.0f);

	const FVector3 m_basisUp = FVector3(0.0f, 1.0f, 0.0f);

	bool m_cinematic = true;

public:
	Camera(std::string name, Object* parent, UINT width, UINT height);

	~Camera();

	XMMATRIX GetViewMatrix();

	inline XMMATRIX GetProjectionMatrix() const { return m_projectionMatrix; }

	//inline FVector3 GetPosition() const { return m_cameraPosition; }

	inline const float GetPixelStepScale() const { return m_pixelStepScale; }

	inline const UINT GetWidth() const { return m_width; }

	inline const UINT GetHeight() const { return m_height; }

protected:
	void CalculateViewMatrix();
};