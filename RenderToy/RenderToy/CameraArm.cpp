#include "CameraArm.h"

CameraArm::CameraArm(std::string name) 
	: SceneObjectComponent(name){}

CameraArm::~CameraArm() {}

void CameraArm::AttachComponent(std::unique_ptr<SceneObjectComponent> component)
{
	// Only one camera component can be attached to a camera arm
	if (!GetComponents().empty())
	{
		return;
	}

	SceneObjectComponent* pComponent = component.get();
	if (!pComponent)
	{
		return;
	}

	Camera* cameraComponent = dynamic_cast<Camera*>(pComponent);

	if (cameraComponent == nullptr)
	{
		return;
	}

	SceneObjectComponent::AttachComponent(std::move(component));
}