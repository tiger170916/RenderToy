#include "Camera.h"
#include "GraphicsUtils.h"

Camera::Camera(std::string name, SceneObjectComponent* parent, UINT width, UINT height)
	: SceneObjectComponent(name, parent), m_width(width), m_height(height)
{
	float fov_y = 0.25f * DirectX::XM_PI;
	float aspectRatio = (float)m_width / (float)m_height;
	float nearPlane = 0.5f;
	float farPlane = 1000.0f;
	m_projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(fov_y, aspectRatio, nearPlane, farPlane);
	m_pixelStepScale = tan(fov_y * 0.5f) * aspectRatio * 2.0f / (float)width;
}

void Camera::CalculateViewMatrix()
{
	FVector3 forwardDir;
	m_viewMatrix = GraphicsUtils::ViewMatrixFromPositionRotation(m_cameraPosition, m_cameraRotation, forwardDir);
}

void Camera::CalculateViewMatrixWithLookAtPosition()
{
	m_viewMatrix = XMMatrixLookAtLH(
		XMVectorSet(m_cameraPosition.X, m_cameraPosition.Y, m_cameraPosition.Z, 1.0f),
		XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),
		XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f));
}


Camera::~Camera()
{

}