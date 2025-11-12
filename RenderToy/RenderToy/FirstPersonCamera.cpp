#include "FirstPersonCamera.h"
#include "GraphicsUtils.h"

FirstPersonCamera::FirstPersonCamera(std::string name, SceneObjectComponent* parent, UINT width, UINT height)
	: Camera(name, parent, width, height)
{

}

void FirstPersonCamera::SetInitialPositionAndRotation(FVector3 initPosition, FRotator initRotation)
{
	m_cameraPosition = initPosition;
	m_cameraRotation = initRotation;
	CalculateViewMatrix();
}

void FirstPersonCamera::ProcessInput(DirectX::Mouse::State mouseState, DirectX::Keyboard::State keyboardState, float deltaTime)
{
	bool changed = false;
	if (mouseState.x != 0 || mouseState.y != 0)
	{
		std::string msg = "first person camera: mouse X: " + std::to_string(mouseState.x);
		msg = msg + " mouse Y: " + std::to_string(mouseState.y);
		msg = msg + " delta:" + std::to_string(deltaTime);
		OutputDebugStringA(msg.c_str());

		m_cameraRotation.Yaw += mouseState.x * deltaTime;
		m_cameraRotation.Pitch += mouseState.y * deltaTime;
		changed = true;
	}

	int forward = 0;
	int right = 0;
	if (keyboardState.W)
	{
		forward += 1;
	}

	if (keyboardState.S)
	{
		forward -= 1;
	}

	if (keyboardState.D)
	{
		right += 1;
	}

	if (keyboardState.A)
	{
		right -= 1;
	}

	if (forward != 0 || right != 0)
	{
		FVector3 forwardDir;
		FVector3 rightDir;
		FVector3 upDir;
		GraphicsUtils::GetForwardRightUpVectorFromRotator(m_cameraRotation, forwardDir, rightDir, upDir);
		m_cameraPosition = m_cameraPosition + forwardDir * (float)forward * deltaTime * 10.0f + rightDir * (float)right * deltaTime * 10.0f;
		changed = true;
	}

	if (changed)
	{
		CalculateViewMatrix();
	}
}