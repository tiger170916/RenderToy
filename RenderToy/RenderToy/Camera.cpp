#include "Camera.h"
#include "GraphicsUtils.h"

Camera::Camera(UINT width, UINT height, FVector3 initPosition, FRotator initRotation)
	: m_width(width), m_height(height), m_position(initPosition), m_rotator(initRotation)
{
	float fov_y = 0.25f * DirectX::XM_PI;
	float aspectRatio = (float)m_width / (float)m_height;
	float nearPlane = 0.5f;
	float farPlane = 1000.0f;
	m_projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(fov_y, aspectRatio, nearPlane, farPlane);
	CalculateViewMatrix();
	float a = tan(fov_y * 0.5f);
	//m_pixelStepScale = 2.0f * tanf(0.25f * DirectX::XM_PI / (2.0f * (float)width));
	m_pixelStepScale = tan(fov_y * 0.5f) * aspectRatio * 2.0f / (float)width;
}

void Camera::CalculateViewMatrix()
{
	FVector3 forwardDir;
	m_viewMatrix = GraphicsUtils::ViewMatrixFromPositionRotation(m_position, m_rotator, forwardDir);
}

void Camera::CalculateViewMatrixWithLookAtPosition()
{
	m_viewMatrix = XMMatrixLookAtLH(
		XMVectorSet(m_position.X, m_position.Y, m_position.Z, 1.0f),
		XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),
		XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f));
}

void Camera::Frame(float delta)
{
	static float curPos = 0.0f;
	static const float speed = 0.2f;

	curPos += delta * speed;
	m_position.X = cos(curPos) * 20.0f;
	m_position.Z = sin(curPos) * 20.0f;

	CalculateViewMatrixWithLookAtPosition();
}

void Camera::ProcessInput(DirectX::Mouse::State mouseState, DirectX::Keyboard::State keyboardState, float deltaTime)
{
	bool changed = false;
	if (mouseState.x != 0 || mouseState.y != 0)
	{
		std::string msg = "mouse X: " + std::to_string(mouseState.x);
		msg = msg + " mouse Y: " + std::to_string(mouseState.y);
		msg = msg + " delta:" + std::to_string(deltaTime);
		OutputDebugStringA(msg.c_str());

		m_rotator.Yaw += mouseState.x  * deltaTime;
		m_rotator.Pitch += mouseState.y * deltaTime;
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
		GraphicsUtils::GetForwardRightUpVectorFromRotator(m_rotator, forwardDir, rightDir, upDir);
		m_position = m_position + forwardDir * (float)forward * deltaTime * 10.0f + rightDir * (float)right * deltaTime * 10.0f;
		changed = true;
	}

	if (changed)
	{
		CalculateViewMatrix();
	}
}

Camera::~Camera()
{

}