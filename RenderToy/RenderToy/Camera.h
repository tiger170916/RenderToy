#pragma once

#include "Includes.h"
#include "Vectors.h"
#include "IControllable.h"

class Camera : public IControllable
{
private:
	FVector3 m_position = FVector3::Zero();

	FRotator m_rotator = FRotator::Zero();

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
	// IControllable interface
	virtual void ProcessInput(DirectX::Mouse::State mouseState, DirectX::Keyboard::State keyboardState, float deltaTime) override;

public:
	Camera(UINT width, UINT height, FVector3 initPosition, FRotator initRotation);

	~Camera();

	inline XMMATRIX GetViewMatrix() const { return m_viewMatrix; }

	inline XMMATRIX GetProjectionMatrix() const { return m_projectionMatrix; }

	inline FVector3 GetPosition() const { return m_position; }

	void Frame(float delta);

	inline const float GetPixelStepScale() const { return m_pixelStepScale; }

	inline const UINT GetWidth() const { return m_width; }

	inline const UINT GetHeight() const { return m_height; }

private:
	void CalculateViewMatrix();

	void CalculateViewMatrixWithLookAtPosition();
};