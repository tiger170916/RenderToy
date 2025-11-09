#pragma once

#include "SceneObject.h"
#include "IControllable.h"

class PlayableCharacterObject : public SceneObject, public IControllable
{
public:
	virtual void ProcessInput(DirectX::Mouse::State, DirectX::Keyboard::State, float deltaTime) override;

	PlayableCharacterObject(std::string name);
};