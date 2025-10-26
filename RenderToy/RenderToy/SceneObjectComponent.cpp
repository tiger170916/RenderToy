#include "SceneObjectComponent.h"

SceneObjectComponent::SceneObjectComponent() {}

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
	for (auto& comp : sceneObjectComponent->m_childrenComponents)
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
	m_childrenComponents.push_back(std::move(component));
}

const Transform SceneObjectComponent::GetTransform() const
{
	Transform totalTransform = m_transform;

	SceneObjectComponent* parent = m_parent;
	while (parent != nullptr)
	{
		Transform parentTransform = parent->GetTransform();
		totalTransform.Rotation = parentTransform.Rotation + totalTransform.Rotation;
		totalTransform.Translation = parentTransform.Translation + totalTransform.Translation;
	}

	return totalTransform;
}