#pragma once
#include "Includes.h"
#include "SceneObjectComponent.h"
#include "Object.h"

class SceneObject : public Object
{
protected:
	std::string m_name;

	std::unique_ptr<SceneObjectComponent> m_rootComponent;

public:
	void SetRootComponent(std::unique_ptr<SceneObjectComponent> component) { m_rootComponent = std::move(component); }

	inline std::string GetName() const { return m_name; }

	SceneObjectComponent* GetRootComponent() { return m_rootComponent.get(); }

	/// <summary>
	/// Tick function for game logic
	/// </summary>
	/// <param name="delta"></param>
	virtual void Tick(float delta) = 0;

protected:
	SceneObject(std::string name);
};
