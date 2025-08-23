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

Camera::~Camera()
{

}