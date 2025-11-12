#include "ThirdPersonCamera.h"
#include "GraphicsUtils.h"
#include "CameraArm.h"

ThirdPersonCamera::ThirdPersonCamera(std::string name, SceneObjectComponent* parent, UINT width, UINT height)
	:Camera(name, parent, width, height)
{
	if (parent)
	{
		const Transform parentTrans = parent->GetTransform();
		m_cameraPosition += parentTrans.Translation;
		m_cameraRotation += parentTrans.Rotation;
		CameraArm* arm = (CameraArm*)dynamic_cast<CameraArm*>(parent);
		if (arm)
		{
			m_basisForward;
			XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(m_cameraRotation.Pitch, m_cameraRotation.Yaw, m_cameraRotation.Roll);

			XMVECTOR forwardDirXM = XMVector3Transform(XMVectorSet(0.0f, 0.0f, -1.0f, 1.0f), rotationMatrix);
			m_cameraPosition.X += forwardDirXM.m128_f32[0];
			m_cameraPosition.Y += forwardDirXM.m128_f32[1];
			m_cameraPosition.Z += forwardDirXM.m128_f32[2];
		}
	}

	CalculateViewMatrix();
}

void ThirdPersonCamera::ProcessInput(DirectX::Mouse::State mouseState, DirectX::Keyboard::State keyboardState, float deltaTime)
{
	if (m_parent)
	{
		const Transform parentTrans = m_parent->GetTransform();
		m_cameraPosition += parentTrans.Translation;
		m_cameraRotation += parentTrans.Rotation;
		CameraArm* arm = (CameraArm*)dynamic_cast<CameraArm*>(m_parent);
		if (arm)
		{
			m_basisForward;
			XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(m_cameraRotation.Pitch, m_cameraRotation.Yaw, m_cameraRotation.Roll);

			XMVECTOR forwardDirXM = XMVector3Transform(XMVectorSet(0.0f, 0.0f, -1.0f, 1.0f), rotationMatrix);
			m_cameraPosition.X += forwardDirXM.m128_f32[0];
			m_cameraPosition.Y += forwardDirXM.m128_f32[1];
			m_cameraPosition.Z += forwardDirXM.m128_f32[2];

		}
	}

	CalculateViewMatrix();
}
