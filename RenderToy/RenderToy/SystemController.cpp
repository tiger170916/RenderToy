#include "SystemController.h"

void SystemController::ProcessInput(DirectX::Mouse::State mouse, DirectX::Keyboard::State keyboard, float deltaTime)
{
	// Switch camera
	if (keyboard.P)
	{
		if (m_world)
		{
			m_world->SwitchCamera();
		}
	}
}