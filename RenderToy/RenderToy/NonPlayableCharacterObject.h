#pragma once

#include "SceneObject.h"

/// <summary>
/// None playable character object
/// </summary>
class NonPlayableCharacterObject : public SceneObject
{
public:
	NonPlayableCharacterObject(std::string name);

	virtual void Tick(float delta) override;
};