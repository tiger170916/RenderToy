#pragma once
#include "Includes.h"
#include "Vectors.h"
#include "ResourceCompilerTile.h"

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

protected:
	std::vector<std::unique_ptr<SceneObjectComponent>> m_components;

	SceneObjectComponent* m_parent = nullptr;

	Transform m_transform = Transform::Identity();

	std::string m_name;


public:

	virtual ~SceneObjectComponent();

	const std::vector<std::unique_ptr<SceneObjectComponent>>& GetComponents() { return m_components; }

	void AttachComponent(std::unique_ptr<SceneObjectComponent> component);

	inline void SetTransform(Transform& transform) { m_transform = transform; }


	const Transform GetTransform() const;

	inline void SetName(std::string name) { m_name = name; }

	inline std::string GetName() const { return m_name; }

protected:
	SceneObjectComponent(std::string name, SceneObjectComponent* parent);

	SceneObjectComponent(std::string name);

	void SetParent(SceneObjectComponent* parent);
};