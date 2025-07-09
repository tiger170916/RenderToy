#include "Camera.h"

Camera::Camera(UINT width, UINT height, FVector3 initPosition, FRotator initRotation)
	: m_width(width), m_height(height), m_position(initPosition), m_rotator(initRotation)
{
	m_projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(0.25f * DirectX::XM_PI, (float)m_width / (float)m_height, 0.5f, 26620000.0f);
	CalculateViewMatrix();
}

void Camera::CalculateViewMatrix()
{
	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(m_rotator.Pitch, m_rotator.Yaw, m_rotator.Roll);

	XMVECTOR forwardDirXM = XMVector3Transform(XMVectorSet(m_basisForward.X, m_basisForward.Y, m_basisForward.Z, 1.0f), rotationMatrix);
	//XMVECTOR rightDirXM = XMVector3Transform(XMVectorSet(m_basisRight.X, m_basisRight.Y, m_basisRight.Z, 1.0f), rotationMatrix);
	XMVECTOR upDirXM = XMVector3Transform(XMVectorSet(m_basisUp.X, m_basisUp.Y, m_basisUp.Z, 1.0f), rotationMatrix);


	XMVector3Normalize(forwardDirXM);
	//XMVector3Normalize(rightDirXM);
	XMVector3Normalize(upDirXM);

	FVector3 forwardDirection = FVector3(forwardDirXM.m128_f32[0], forwardDirXM.m128_f32[1], forwardDirXM.m128_f32[2]);
	//FVector3 rightDirection = FVector3(rightDirXM.m128_f32[0], rightDirXM.m128_f32[1], rightDirXM.m128_f32[2]);
	FVector3 upDirection = FVector3(upDirXM.m128_f32[0], upDirXM.m128_f32[1], upDirXM.m128_f32[2]);

	FVector3 focusPos = m_position + forwardDirection;

	m_viewMatrix = XMMatrixLookAtLH(
		XMVectorSet(m_position.X, m_position.Y, m_position.Z, 1.0f),
		XMVectorSet(focusPos.X, focusPos.Y, focusPos.Z, 1.0f),
		XMVectorSet(upDirection.X, upDirection.Y, upDirection.Z, 1.0f));
}

Camera::~Camera()
{

}