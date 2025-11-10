#pragma once

#include "Includes.h"
#include "IControllable.h"
#include "World2.h"

class SystemController : public IControllable
{
private:
	World2* m_world = nullptr;

public:
	/// <summary>
	/// IControllable interface implementation
	/// </summary>
	virtual void ProcessInput(DirectX::Mouse::State, DirectX::Keyboard::State, float deltaTime) override;

	void SetActiveWorld(World2* world) { m_world = world; }
};