#pragma once
#include "Includes.h"

class IControllable
{
public:
	virtual void ProcessInput(DirectX::Mouse::State, DirectX::Keyboard::State, float deltaTime) = 0;
};