#pragma once

#include "Camera.h"

class ThirdPersonCamera : public Camera
{
public:
	ThirdPersonCamera(std::string name, SceneObjectComponent* parent, UINT width, UINT height);

	// IControllable interface
	virtual void ProcessInput(DirectX::Mouse::State mouseState, DirectX::Keyboard::State keyboardState, float deltaTime) override;
};