#include "Tile.h"
#include "ExtensionFactory.h"
#include "PlayableCharacterObject.h"
#include "NonPlayableCharacterObject.h"
#include "StaticMeshComponent.h"
#include "CameraArm.h"
#include "ThirdPersonCamera.h"


Tile::Tile(std::string tileName, GraphicsContext* graphicsContext, MaterialManager* materialManager, TextureManager2* texManager, float bboxMinX, float bboxMinY, float bboxMaxX, float bboxMaxY)
	: m_tileName(tileName), m_graphicsContext(graphicsContext), m_materialManager(materialManager), m_textureManager(texManager), m_bboxMinX(bboxMinX), m_bboxMinY(bboxMinY), m_bboxMaxX(bboxMaxX), m_bboxMaxY(bboxMaxY)
{
}

bool Tile::LoadTileContentsFromResource(ResourceCompilerModule::TileData* tileData)
{
	if (!tileData)
	{
		return false;
	}

	ResourceCompilerModule::SceneGraph* sceneGraph = tileData->GetSceneGraph();
	if (!sceneGraph)
	{
		return false;
	}

	// Load None playable characters
	std::vector<ResourceCompilerModule::NonePlayableCharacterObject*> rcNonPlayableObjs;
	if (sceneGraph->GetNonePlayableCharacterObjects(rcNonPlayableObjs))
	{
		for (auto& rcNonPlayableObj : rcNonPlayableObjs)
		{
			ResourceCompilerModule::SceneObjectComponent* rcRootComp = rcNonPlayableObj->GetRootComponent();
			if (!rcRootComp)
			{
				continue;
			}

			std::unique_ptr<NonPlayableCharacterObject> nonPlayableCharacterObject = std::make_unique<NonPlayableCharacterObject>(rcNonPlayableObj->GetName());
			std::unique_ptr<SceneObjectComponent> rootComp = ConstructSceneObjectComponentFromResource(rcNonPlayableObj->GetRootComponent(), nullptr);
			if (rootComp)
			{
				nonPlayableCharacterObject->SetRootComponent(std::move(rootComp));
				m_sceneObjects.push_back(std::move(nonPlayableCharacterObject));
			}
		}
	}

	// Load Playable characters
	std::vector<ResourceCompilerModule::PlayableCharacterObject*> rcPlayableObjs;
	if (sceneGraph->GetPlayableCharacterObjects(rcPlayableObjs))
	{
		for (auto& rcPlayableObj : rcPlayableObjs)
		{
			ResourceCompilerModule::SceneObjectComponent* rcRootComp = rcPlayableObj->GetRootComponent();
			if (!rcRootComp)
			{
				continue;
			}

			std::unique_ptr<PlayableCharacterObject> playableCharacterObject = std::make_unique<PlayableCharacterObject>(rcPlayableObj->GetName());
			std::unique_ptr<SceneObjectComponent> rootComp = ConstructSceneObjectComponentFromResource(rcPlayableObj->GetRootComponent(), nullptr);
			if (rootComp)
			{
				playableCharacterObject->SetRootComponent(std::move(rootComp));
				m_sceneObjects.push_back(std::move(playableCharacterObject));
			}
		}
	}


	return true;
}

std::unique_ptr<SceneObjectComponent> Tile::ConstructSceneObjectComponentFromResource(ResourceCompilerModule::SceneObjectComponent* rcSceneObjectComponent, SceneObjectComponent* parent)
{
	if (!rcSceneObjectComponent)
	{
		return nullptr;
	}

	std::unique_ptr<SceneObjectComponent> result = nullptr;
	switch (rcSceneObjectComponent->GetType())
	{
	case ResourceCompilerModule::SceneObjectComponentType::SCENE_OBJECT_COMPONENT_STATIC_MESH:
	{
		ResourceCompilerModule::StaticMeshComponent* rcMeshComp = (ResourceCompilerModule::StaticMeshComponent*)dynamic_cast<ResourceCompilerModule::StaticMeshComponent*>(rcSceneObjectComponent);
		if (rcMeshComp)
		{
			std::unique_ptr<StaticMeshComponent> staticMeshComponent = std::make_unique<StaticMeshComponent>(rcMeshComp->GetName(), parent);
			staticMeshComponent->ConstructComponentFromResource(rcMeshComp);
			result = std::move(staticMeshComponent);
		}
		break;
	}

	case ResourceCompilerModule::SceneObjectComponentType::SCENE_OBJECT_COMPONENT_CAMERA_ARM:
	{
		ResourceCompilerModule::CameraArmComponent* rcCameraArmComp = (ResourceCompilerModule::CameraArmComponent*)dynamic_cast<ResourceCompilerModule::CameraArmComponent*>(rcSceneObjectComponent);
		if (rcCameraArmComp)
		{
			std::unique_ptr<CameraArm> cameraArm = std::make_unique<CameraArm>(rcSceneObjectComponent->GetName(), parent);
			result = std::move(cameraArm);
		}
		break;
	}
	case ResourceCompilerModule::SceneObjectComponentType::SCENE_OBJECT_COMPONENT_CAMERA:
	{
		ResourceCompilerModule::CameraComponent* rcCameraComp = (ResourceCompilerModule::CameraComponent*)dynamic_cast<ResourceCompilerModule::CameraComponent*>(rcSceneObjectComponent);
		if (rcCameraComp)
		{
			std::unique_ptr<ThirdPersonCamera> camera = std::make_unique<ThirdPersonCamera>(rcSceneObjectComponent->GetName(), parent, m_graphicsContext->GetHwndWidth(), m_graphicsContext->GetHwndHeight());
			result = std::move(camera);
		}
		break;
	}
	}

	// Construct components Recursively
	std::vector<ResourceCompilerModule::SceneObjectComponent*> components;
	rcSceneObjectComponent->GetComponents(components);
	for (auto& comp : components)
	{
		if (!comp)
		{
			continue;
		}

		std::unique_ptr<SceneObjectComponent> component = ConstructSceneObjectComponentFromResource(comp, result.get());
		result->AttachComponent(std::move(component));
	}

	return result;
}

const std::vector<Camera*>& Tile::GetAllCameraComponents()
{
	m_allCameraComponents.clear();
	for (auto& sceneObj : m_sceneObjects)
	{
		if (!sceneObj)
		{
			continue;
		}

		GetComponentsOfTypeInternal(sceneObj->GetRootComponent(), m_allCameraComponents);
	}

	return m_allCameraComponents;
}

const std::vector<StaticMeshComponent*>& Tile::GetAllStaticMeshComponents()
{
	if (!m_staticMeshesDirty)
	{
		return m_allMeshComponents;
	}

	m_allMeshComponents.clear();

	for (auto& sceneObj : m_sceneObjects)
	{
		if (!sceneObj)
		{
			continue;
		}

		GetComponentsOfTypeInternal(sceneObj->GetRootComponent(), m_allMeshComponents);
	}

	m_staticMeshesDirty = false;

	return m_allMeshComponents;
}

template<typename T>
void Tile::GetComponentsOfTypeInternal(SceneObjectComponent* sceneObjectComp, std::vector<T>& array)
{
	if (!sceneObjectComp)
	{
		return;
	}

	T comp = (T)dynamic_cast<T>(sceneObjectComp);
	if (comp)
	{
		array.push_back(comp);
	}

	for (auto& comp : sceneObjectComp->GetComponents())
	{
		GetComponentsOfTypeInternal(comp.get(), array);
	}
}


bool Tile::StreamInBinary(GraphicsContext* graphicsContext, CommandBuilder* cmdBuilder)
{
	if (m_streamedIn)
	{
		return true;
	}

	if (!cmdBuilder)
	{
		return false;
	}

	m_activeMeshes.clear();

	// Stream in static meshes
	for (auto& meshComp : m_allMeshComponents)
	{
		if (!meshComp)
		{
			continue;
		}

		IMesh* mesh = meshComp->GetMesh();
		if (!mesh)
		{
			continue;
		}

		if (mesh->LoadFromBinary(graphicsContext, m_materialManager, m_textureManager, cmdBuilder))
		{
			m_activeMeshes.push_back(mesh);
		}
	}

	m_streamedIn = true;

	return true;
}

bool Tile::StreamOut()
{
	return true;
}

bool Tile::CleanUpAfterStreamIn()
{
	return true;
}

bool Tile::IsPointInTile(float x, float y)
{
	if (x >= m_bboxMinX && x <= m_bboxMaxX &&
		y >= m_bboxMinY && y <= m_bboxMaxY)
	{
		return true;
	}

	return false;
}

void Tile::GetBBox(float& outXMin, float& outXMax, float& outYMin, float& outYMax)
{
	outXMin = m_bboxMinX;
	outXMax = m_bboxMaxX;
	outYMin = m_bboxMinY;
	outYMax = m_bboxMaxY;
}

bool Tile::UpdateBuffersForFrame()
{
	if (!m_streamedIn)
	{
		return false;
	}

	for (auto& mesh : m_activeMeshes)
	{
		mesh->UpdateBuffersForFrame();
	}

	return true;
}