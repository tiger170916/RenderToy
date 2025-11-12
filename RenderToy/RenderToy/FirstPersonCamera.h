#pragma once

#include "Camera.h"

class FirstPersonCamera : public Camera
{
public:
	// IControllable interface
	virtual void ProcessInput(DirectX::Mouse::State mouseState, DirectX::Keyboard::State keyboardState, float deltaTime) override;

	void SetInitialPositionAndRotation(FVector3 initPosition, FRotator initRotation);

	FirstPersonCamera(std::string name, SceneObjectComponent* parent, UINT width, UINT height);
};