#pragma once
#include "Includes.h"
#include "Vectors.h"

class SceneObjectComponent
{
friend class SceneObject;

public:
	class Iterator
	{
	private:
		std::vector<SceneObjectComponent*> m_array;
		SceneObjectComponent* m_sceneObjectComponent = nullptr;
		size_t m_itr = -1;
	public:
		Iterator(SceneObjectComponent* sceneObjectComponent);
		bool Next();
		SceneObjectComponent* GetCurrent();

	private:

		void BuildArray(SceneObjectComponent* sceneObjectComponent);
	};

private:
	std::vector<std::unique_ptr<SceneObjectComponent>> m_childrenComponents;

	SceneObjectComponent* m_parent = nullptr;

	Transform m_transform = Transform::Identity();

public:
	SceneObjectComponent();

	virtual ~SceneObjectComponent();

	const std::vector<std::unique_ptr<SceneObjectComponent>>& GetChildrenComponents() { return m_childrenComponents; }

	void AttachComponent(std::unique_ptr<SceneObjectComponent> component);

	inline void SetTransform(Transform& transform) { m_transform = transform; }

	const Transform GetTransform() const;

protected:
	void SetParent(SceneObjectComponent* parent);
};