#pragma once
#include "Includes.h"
#include "SceneObjectComponent.h"

class SceneObject
{
private:
	std::unique_ptr<SceneObjectComponent> m_rootComponent;

public:
	void SetRootComponent(std::unique_ptr<SceneObjectComponent> component) { m_rootComponent = std::move(component); }

	SceneObjectComponent* GetRootComponent() { return m_rootComponent.get(); }

	 
};
