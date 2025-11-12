#pragma once

#include "Includes.h"

#include "SceneObjectComponent.h"
#include "Camera.h"

class CameraArm : public SceneObjectComponent
{
private:
	float m_length = 5.0f;

public:
	CameraArm(std::string name, SceneObjectComponent* parent);

	~CameraArm();
	
	virtual void AttachComponent(std::unique_ptr<SceneObjectComponent> component);
};