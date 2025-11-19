#include "Camera.h"
#include "GraphicsUtils.h"
#include "CameraArm.h"

Camera::Camera(std::string name, Object* parent, UINT width, UINT height)
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
	// Check if this a third person cam or first person cam.
	CameraArm* camArm = dynamic_cast<CameraArm*>(m_parent);
	Transform transform = GetTransform();
	
	FTranslation camTranslation = transform.Translation;
	// third person
	if (camArm)
	{
		//FVector3 vecForward, vecRight, vecUp;
		//GraphicsUtils::GetForwardRightUpVectorFromRotator(transform.Rotation, vecForward, vecRight, vecUp);

		XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(transform.Rotation.Pitch, transform.Rotation.Yaw, transform.Rotation.Roll);
		XMVECTOR forwardDirXM = XMVector3Transform(XMVectorSet(0.0f, 0.0f, -1.0f, 1.0f), rotationMatrix);

		// arm length is cosnt 2.0 for temp
		camTranslation.X += forwardDirXM.m128_f32[0] * 2.0f;
		camTranslation.Y += forwardDirXM.m128_f32[1] * 2.0f;
		camTranslation.Z += forwardDirXM.m128_f32[2] * 2.0f;
	}

	FVector3 forwardDir;
	m_viewMatrix = GraphicsUtils::ViewMatrixFromPositionRotation(camTranslation, transform.Rotation, forwardDir);
}

Camera::~Camera()
{

}

XMMATRIX Camera::GetViewMatrix()
{
	CalculateViewMatrix();
	return m_viewMatrix;
}