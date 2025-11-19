#pragma once

#include "InputComponent.h"

/// <summary>
/// Input component class for standalone first person camera
/// </summary>
class StandAloneCameraInputComponent : public InputComponent
{
public:
	virtual void ProcessInput(DirectX::Mouse::State, DirectX::Keyboard::State, float deltaTime) override;
};