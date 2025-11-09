#pragma once
#include "Includes.h"
#include "SceneObjectComponent.h"

class SceneObject
{
private:
	std::string m_name;

	std::unique_ptr<SceneObjectComponent> m_rootComponent;

public:
	void SetRootComponent(std::unique_ptr<SceneObjectComponent> component) { m_rootComponent = std::move(component); }

	inline std::string GetName() const { return m_name; }

	SceneObjectComponent* GetRootComponent() { return m_rootComponent.get(); }



protected:
	SceneObject(std::string name);
};
