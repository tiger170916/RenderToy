#pragma once

#include "SceneObject.h"
#include "IControllable.h"

class PlayableCharacterObject : public SceneObject, public IControllable
{
private:
	// speed vector of the character
	FVector2 m_velocity = {};

	FVector2 m_mouseVelocity = {};
public:
	virtual void ProcessInput(InputStruct inputStruct) override;

	virtual void Tick(float delta) override;

	PlayableCharacterObject(std::string name);
};