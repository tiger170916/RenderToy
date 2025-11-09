#pragma once

#include "Includes.h"
#include "SceneObject.h"
#include "IControllable.h"

class Character : public SceneObject, public IControllable
{
private:

public:
	// Implement the IControllable interface
	virtual void ProcessInput(DirectX::Mouse::State, DirectX::Keyboard::State, float deltaTime) override;


};