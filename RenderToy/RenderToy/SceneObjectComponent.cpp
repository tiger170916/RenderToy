#include "SceneObjectComponent.h"

SceneObjectComponent::SceneObjectComponent(std::string name, Object* parent) 
	: m_name(name), m_parent(parent)
{
	assert(parent != nullptr);
}

SceneObjectComponent::~SceneObjectComponent() {}

SceneObjectComponent::Iterator::Iterator(SceneObjectComponent* sceneObjectComponent)
{
	m_sceneObjectComponent = sceneObjectComponent;

	BuildArray(sceneObjectComponent);
}

bool SceneObjectComponent::Iterator::Next()
{
	return ++m_itr < m_array.size();
}

SceneObjectComponent* SceneObjectComponent::Iterator::GetCurrent()
{
	if (m_itr >= m_array.size() && m_itr > -1)
	{
		return nullptr;
	}
	return m_array[m_itr];
}

void SceneObjectComponent::Iterator::BuildArray(SceneObjectComponent* sceneObjectComponent)
{
	if (!sceneObjectComponent)
	{
		return;
	}

	m_array.push_back(sceneObjectComponent);
	for (auto& comp : sceneObjectComponent->m_components)
	{
		BuildArray(comp.get());
	}
}

void SceneObjectComponent::SetParent(SceneObjectComponent* parent)
{
	m_parent = parent;
}

void SceneObjectComponent::AttachComponent(std::unique_ptr<SceneObjectComponent> component)
{
	if (!component)
	{
		return;
	}

	component->SetParent(this);
	m_components.push_back(std::move(component));
}

const Transform SceneObjectComponent::GetTransform() const
{
	Transform totalTransform = m_transform;

	SceneObjectComponent* parent = dynamic_cast<SceneObjectComponent*>(m_parent);
	while (parent != nullptr)
	{
		Transform parentTransform = parent->GetTransform();
		totalTransform.Rotation = parentTransform.Rotation + m_transform.Rotation;
		totalTransform.Translation = parentTransform.Translation + m_transform.Translation;
		parent = dynamic_cast<SceneObjectComponent*>(parent->GetParent());
	}

	return totalTransform;
}

bool SceneObjectComponent::IsRootComponent() const
{
	SceneObjectComponent* sceneObj = dynamic_cast<SceneObjectComponent*>(m_parent);
	return sceneObj == nullptr;
}

Object* SceneObjectComponent::GetSceneObject()
{
	SceneObjectComponent* parent = dynamic_cast<SceneObjectComponent*>(m_parent);
	while (parent != nullptr)
	{
		parent = dynamic_cast<SceneObjectComponent*>(parent->GetParent());
	}

	return m_parent;
}