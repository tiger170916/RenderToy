#pragma once
#include "IControllable.h"

class InputComponent
{
public:
	virtual void ProcessInput(DirectX::Mouse::State, DirectX::Keyboard::State, float deltaTime) = 0;
};