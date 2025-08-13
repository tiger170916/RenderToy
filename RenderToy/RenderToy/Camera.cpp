#include "Camera.h"
#include "GraphicsUtils.h"

Camera::Camera(UINT width, UINT height, FVector3 initPosition, FRotator initRotation)
	: m_width(width), m_height(height), m_position(initPosition), m_rotator(initRotation)
{
	m_projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(0.25f * DirectX::XM_PI, (float)m_width / (float)m_height, 0.5f, 1000.0f);
	CalculateViewMatrix();
}

void Camera::CalculateViewMatrix()
{
	m_viewMatrix = GraphicsUtils::ViewMatrixFromPositionRotation(m_position, m_rotator);
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
	static const float speed = 0.1f;

	curPos += delta * speed;
	m_position.X = cos(curPos) * 35.0f;
	m_position.Z = sin(curPos) * 35.0f;

	CalculateViewMatrixWithLookAtPosition();
}

Camera::~Camera()
{

}