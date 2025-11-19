#include "PlayableCharacterObject.h"
#include "GraphicsUtils.h"

PlayableCharacterObject::PlayableCharacterObject(std::string name)
	: SceneObject(name)
{

}

void PlayableCharacterObject::Tick(float delta)
{
	Transform transform = m_rootComponent->GetTransform();
	if (m_velocity.X != 0.0f || m_velocity.Y != 0.0f)
	{
		FVector3 forwardVec, rightVec, upVec;
		GraphicsUtils::GetForwardRightUpVectorFromRotator(
			transform.Rotation,
			forwardVec,
			rightVec,
			upVec);

		transform.Translation += (forwardVec * m_velocity.X + rightVec * m_velocity.Y) * delta;
	}

	if (m_mouseVelocity.X != 0 || m_mouseVelocity.Y != 0)
	{
		transform.Rotation.Yaw += m_mouseVelocity.X * delta;
		transform.Rotation.Pitch = std::clamp(transform.Rotation.Pitch + m_mouseVelocity.Y * delta, -1.5f, 1.5f);
	}

	m_rootComponent->SetTransform(transform);
}

void PlayableCharacterObject::ProcessInput(InputStruct inputStruct)
{
	m_velocity.X = inputStruct.ForwardAxis;
	m_velocity.Y = inputStruct.RightAxis;

	m_mouseVelocity.X = inputStruct.MouseXAxis;
	m_mouseVelocity.Y = inputStruct.MouseYAxis;
}

