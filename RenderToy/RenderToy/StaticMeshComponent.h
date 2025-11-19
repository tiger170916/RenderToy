#pragma once

#include "SceneObjectComponent.h"
#include "ResourceCompilerTile.h"
#include "IMesh.h"

class StaticMeshComponent : public SceneObjectComponent
{
private:
	std::unique_ptr<IMesh> m_mesh;

public:
	StaticMeshComponent(std::string name, Object* pParent);

	virtual bool ConstructComponentFromResource(ResourceCompilerModule::StaticMeshComponent* rcStaticMeshComponent);

	inline IMesh* GetMesh() { return m_mesh.get(); }
};