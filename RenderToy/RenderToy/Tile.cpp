#include "Tile.h"
#include "ExtensionFactory.h"
#include "PlayableCharacterObject.h"
#include "NonPlayableCharacterObject.h"
#include "StaticMeshComponent.h"
#include "CameraArm.h"


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
			std::unique_ptr<SceneObjectComponent> rootComp = ConstructSceneObjectComponentFromResource(rcNonPlayableObj->GetRootComponent());
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
			std::unique_ptr<SceneObjectComponent> rootComp = ConstructSceneObjectComponentFromResource(rcPlayableObj->GetRootComponent());
			if (rootComp)
			{
				playableCharacterObject->SetRootComponent(std::move(rootComp));
				m_sceneObjects.push_back(std::move(playableCharacterObject));
			}
		}
	}


	return true;
}

std::unique_ptr<SceneObjectComponent> Tile::ConstructSceneObjectComponentFromResource(ResourceCompilerModule::SceneObjectComponent* rcSceneObjectComponent)
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
			std::unique_ptr<StaticMeshComponent> staticMeshComponent = std::make_unique<StaticMeshComponent>(rcMeshComp->GetName(), nullptr);
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
			std::unique_ptr<CameraArm> cameraArm = std::make_unique<CameraArm>(rcSceneObjectComponent->GetName());
			result = std::move(cameraArm);
		}
		break;
	}
	case ResourceCompilerModule::SceneObjectComponentType::SCENE_OBJECT_COMPONENT_CAMERA:
	{
		ResourceCompilerModule::CameraComponent* rcCameraComp = (ResourceCompilerModule::CameraComponent*)dynamic_cast<ResourceCompilerModule::CameraComponent*>(rcSceneObjectComponent);
		if (rcCameraComp)
		{
			std::unique_ptr<Camera> camera = std::make_unique<Camera>(rcSceneObjectComponent->GetName(), m_graphicsContext->GetHwndWidth(), m_graphicsContext->GetHwndHeight(), FVector3(0.0f, 0.0f, 0.0f), FRotator(0.0f, 0.0f, 0.0f));
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

		std::unique_ptr<SceneObjectComponent> component = ConstructSceneObjectComponentFromResource(comp);
		result->AttachComponent(std::move(component));
	}

	return result;
}

/*
bool Tile::LoadTileContentsFromFile(std::filesystem::path filePath)
{
	std::filesystem::path rootDir = filePath.parent_path();
	
	bool loadSucc = false;
	std::fstream file(filePath, std::ios::in | std::ios::binary);
	if (file.is_open())
	{
		ResourceCompilerModule::BinaryHeader binaryHeader = {};
		// Only accepts binary type headers
		if (ReadHeader(&file, binaryHeader) && binaryHeader.BinaryType == ResourceCompilerModule::BinaryType::BINARY_TYPE_TILE)
		{
			ResourceCompilerModule::StaticMeshHeader* staticMeshHeaders = nullptr;
			ResourceCompilerModule::StaticMeshInstanceHeader* instanceHeaders = nullptr;
			ResourceCompilerModule::LightExtensionHeader* lightExtensionHeaders = nullptr;

			uint32_t numStaticMeshes = 0;
			uint32_t numInstances = 0;
			uint32_t numLightExtensions = 0;

			loadSucc = GetStaticMeshInstances(&file, binaryHeader, &instanceHeaders, numInstances) &&
							GetStaticMeshes(&file, binaryHeader, &staticMeshHeaders, numStaticMeshes) &&
							GetLightExtensions(&file, binaryHeader, &lightExtensionHeaders, numLightExtensions);
			
			
			if (loadSucc)
			{
				for (uint32_t i = 0; i < numStaticMeshes; i++)
				{
					ResourceCompilerModule::StaticMeshHeader& staticMeshHeader = staticMeshHeaders[i];
					const uint32_t instanceCount = staticMeshHeader.InstanceCount;
					std::unique_ptr<IMesh> mesh = nullptr;
					if (instanceCount == 1)
					{
						mesh = std::make_unique<StaticMesh2>(staticMeshHeader.MeshName);
						StaticMesh2* staticMesh = (StaticMesh2*)mesh.get();

						ResourceCompilerModule::StaticMeshInstanceHeader& instanceHeader = instanceHeaders[staticMeshHeader.InstanceIndex];

						// Read transfrom 
						Transform transform = {};
						memcpy(&transform.Rotation, instanceHeader.Rotation, sizeof(FRotator));
						memcpy(&transform.Translation, instanceHeader.Position, sizeof(FTranslation));
						memcpy(&transform.Scale, instanceHeader.Scale, sizeof(FScale));
						staticMesh->SetTransform(transform);

						// Create light extension if it has
						if (instanceHeader.HasLightExtension)
						{
							ResourceCompilerModule::LightExtensionHeader& lightExtHeader = lightExtensionHeaders[instanceHeader.LightExtensionIndex];
							std::unique_ptr<LightExtension> lightExt = ExtensionFactory::SpawnLightExtension(
								lightExtHeader.Offset,
								lightExtHeader.Rotation,
								lightExtHeader.Color,
								lightExtHeader.Intensity,
								lightExtHeader.AttenuationRadius,
								lightExtHeader.AspectRatio,
								lightExtHeader.Fov);

							staticMesh->AttachLightExtension(std::move(lightExt));
						}
					}
					else 
					{
						mesh = std::make_unique<InstancedStaticMesh>(staticMeshHeader.MeshName);
						InstancedStaticMesh* instancedStaticMesh = (InstancedStaticMesh*)mesh.get();
						// Get instances of this mesh
						for (uint32_t instanceIdx = 0; instanceIdx < staticMeshHeader.InstanceCount; instanceIdx++)
						{
							ResourceCompilerModule::StaticMeshInstanceHeader& instanceHeader = instanceHeaders[instanceIdx + staticMeshHeader.InstanceIndex];

							Transform transform = {};
							memcpy(&transform.Rotation, instanceHeader.Rotation, sizeof(FRotator));
							memcpy(&transform.Translation, instanceHeader.Position, sizeof(FTranslation));
							memcpy(&transform.Scale, instanceHeader.Scale, sizeof(FScale));
							instancedStaticMesh->AddInstance(transform);

							// Create light extension if it has
							if (instanceHeader.HasLightExtension)
							{
								ResourceCompilerModule::LightExtensionHeader& lightExtHeader = lightExtensionHeaders[instanceHeader.LightExtensionIndex];
								std::unique_ptr<LightExtension> lightExt = ExtensionFactory::SpawnLightExtension(
									lightExtHeader.Offset,
									lightExtHeader.Rotation,
									lightExtHeader.Color,
									lightExtHeader.Intensity,
									lightExtHeader.AttenuationRadius,
									lightExtHeader.AspectRatio,
									lightExtHeader.Fov);
								instancedStaticMesh->AttachLightExtension(instanceIdx, std::move(lightExt));
							}
						}
					}

					mesh->SetIsFromFile(rootDir / staticMeshHeader.FileName);
	
					std::unique_ptr<SceneObject> sceneObject = std::make_unique<SceneObject>();
					std::unique_ptr<SceneObjectComponent> sceneObjectComp = std::move(mesh);
					sceneObject->SetRootComponent(std::move(sceneObjectComp));

					m_sceneObjects.push_back(std::move(sceneObject));
				}
			}

			if (numStaticMeshes > 0)
			{
				delete[] staticMeshHeaders;
			}
			if (numInstances > 0)
			{
				delete[] instanceHeaders;
			}
			if (numLightExtensions > 0)
			{
				delete[] lightExtensionHeaders;
			}
		}
	}

	file.close();
	
	return loadSucc;
return false;
}*/

/*
void Tile::LoadMeshFromFile(
	std::filesystem::path filePath,
	ResourceCompilerModule::StaticMeshDefinitionHeader* staticMeshDefinitionHeaders, ResourceCompilerModule::MeshPartHeader* meshPartHeaders,
	ResourceCompilerModule::MaterialHeader* materialHeaders,
	uint32_t numStaticMeshDefinitions,
	uint32_t numMeshParts,
	uint32_t numMaterials)
{
	for (auto& itr : m_sceneObjects)
	{
		SceneObject* sceneObject = itr.get();
		if (sceneObject == nullptr)
		{
			continue;
		}

		SceneObjectComponent* rootComponent = sceneObject->GetRootComponent();
		SceneObjectComponent::Iterator itr(rootComponent);
		while (itr.Next())
		{
			SceneObjectComponent* sceneObjComponent = itr.GetCurrent();
			IMesh* mesh = dynamic_cast<IMesh*>(sceneObjComponent);

			if (mesh)
			{
				try {
					if (std::filesystem::equivalent(mesh->GetFilePath(), filePath))
					{
						// Try to find the parts in the file
						for (uint32_t meshDefIdx = 0; meshDefIdx < numStaticMeshDefinitions; meshDefIdx++)
						{
							ResourceCompilerModule::StaticMeshDefinitionHeader& meshDefHeader = staticMeshDefinitionHeaders[meshDefIdx];
							if (meshDefHeader.MeshName == mesh->GetName())
							{
								// found the mesh definition.
								for (uint32_t meshPartIdx = meshDefHeader.PartsIdx; meshPartIdx < meshDefHeader.PartsIdx + meshDefHeader.NumParts; meshPartIdx++)
								{
									ResourceCompilerModule::MeshPartHeader& meshPartHeader = meshPartHeaders[meshPartIdx];
									mesh->AddFileMetadataPart(meshPartHeader.MeshDataOffset, meshPartHeader.MeshDataSize, materialHeaders[meshPartHeader.MaterialIdx].MaterialName);
								}

								break;
							}
						}
					}
				}
				catch (std::exception)
				{
					continue;
				}
			}
		}
		
	}
}*/

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

	for (auto& itr : m_sceneObjects)
	{
		SceneObject* sceneObject = itr.get();
		if (sceneObject == nullptr)
		{
			continue;
		}

		SceneObjectComponent* rootComponent = sceneObject->GetRootComponent();
		SceneObjectComponent::Iterator itr(rootComponent);
		while (itr.Next())
		{
			SceneObjectComponent* sceneObjComponent = itr.GetCurrent();
			IMesh* mesh = dynamic_cast<IMesh*>(sceneObjComponent);

			if (!mesh)
			{
				continue;
			}
			if (mesh->LoadFromBinary(graphicsContext, m_materialManager, m_textureManager, cmdBuilder))
			{
				m_activeMeshes.push_back(mesh);
			}

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