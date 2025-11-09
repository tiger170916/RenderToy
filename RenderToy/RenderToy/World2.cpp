#include "World2.h"
#include "ResourceCompilerModuleApi.h"
#include "ResourceCompilerTile.h"
#include "ResourceLoader.h"

World2::World2(GraphicsContext* graphicsContext, MaterialManager* materialManager, TextureManager2* texManager)
	: m_graphicsContext(graphicsContext), m_materialManager(materialManager), m_textureManager(texManager) {
}

bool World2::Initialize(GraphicsContext* graphicsContext)
{
	if (!graphicsContext)
	{
		return false;
	}
	
	if (!m_uniformFrameConstantBuffer)
	{
		m_uniformFrameConstantBuffer = std::make_unique<ConstantBuffer<UniformFrameConstants>>();
		if (!m_uniformFrameConstantBuffer->Initialize(graphicsContext))
		{
			return false;
		}
	}

	if (!m_lightConstantBuffer)
	{
		m_lightConstantBuffer = std::make_unique<ConstantBuffer<LightConstantsDx>>();
		if (!m_lightConstantBuffer->Initialize(graphicsContext))
		{
			return false;
		}
	}
	return true;
}

World2::~World2() {}

void World2::CreateStandaloneCamera(UINT width, UINT height, FVector3 position, FRotator rotator)
{
	std::string camName = "StandAloneCamera" + std::to_string(m_standaloneCameras.size());
	std::string characterName = camName + "Character";
	std::unique_ptr<PlayableCharacterObject> standAloneCameraCharacter = std::make_unique<PlayableCharacterObject>(characterName);
	std::unique_ptr<Camera> cameraComponent = std::make_unique<Camera>(camName, width, height, position, rotator);
	
	standAloneCameraCharacter->SetRootComponent(std::move(cameraComponent));

	m_standaloneCameras.push_back(std::move(standAloneCameraCharacter));
}

Camera* World2::GetActiveCamera()
{
	GetAllCamerasInternal();
	if (m_activeCameraIdx < 0 || m_activeCameraIdx >= (int)m_allCameras.size())
	{
		m_activeCameraIdx = (int)m_allCameras.size() - 1;
	}

	if (m_activeCameraIdx < 0)
	{
		return nullptr;
	}

	return m_allCameras[m_activeCameraIdx];
}

void World2::GetAllCamerasInternal()
{
	if (m_cameraListDirty)
	{
		m_allCameras.clear();
		// Standalone cameras
		for (auto& standaloneCamera : m_standaloneCameras)
		{
			if (!standaloneCamera)
			{
				continue;
			}

			SceneObjectComponent* rootComp = standaloneCamera->GetRootComponent();
			Camera* camera = (Camera*)dynamic_cast<Camera*>(rootComp);
			if (camera)
			{
				m_allCameras.push_back(camera);
			}
		}

		// Component cameras
		for (auto& tile : m_tiles)
		{
			if (!tile)
			{
				continue;
			}

			const std::vector<Camera*>& cameraComponents = tile->GetAllCameraComponents();
			m_allCameras.insert(m_allCameras.end(), cameraComponents.begin(), cameraComponents.end());
		}

		m_cameraListDirty = true;
	}
}

bool World2::LoadFromBinary(std::filesystem::path rootDirectory)
{
	if (std::filesystem::exists(rootDirectory) && std::filesystem::is_directory(rootDirectory)) {

		// Load tiles
		for (const auto& entry : std::filesystem::directory_iterator(rootDirectory))
		{
			std::filesystem::path filePath = entry.path();
			if (filePath.extension() != ".bin")
			{
				continue;
			}

			ResourceCompilerModule::ResourceType binType = ResourceCompilerModule::GetResourceType(filePath.string().c_str());
			if (binType != ResourceCompilerModule::ResourceType::RESOURCE_TYPE_TILE)
			{
				continue;
			}

			ResourceCompilerModule::ResourceLoader* loader = ResourceCompilerModule::LoadFile(filePath.string().c_str());
			ResourceCompilerModule::TileData* tileData = loader->GetTileData();
			// Create the tile
			if (tileData)
			{
				float bboxMinX = 0.0f, bboxMinY = 0.0f, bboxMaxX = 0.0f, bboxMaxY = 0.0f;
				tileData->GetBoundingBox(bboxMinX, bboxMinY, bboxMaxX, bboxMaxY);

				std::unique_ptr<Tile> tile = std::make_unique<Tile>(
					tileData->GetTileName(), m_graphicsContext, m_materialManager, m_textureManager, bboxMinX, bboxMinY, bboxMaxX, bboxMaxY);

				// Load from bin
				tile->LoadTileContentsFromResource(tileData);

				m_tiles.push_back(std::move(tile));
			}

			delete loader;
		}

		// Load all meshes & materials
		std::map<ResourceCompilerModule::ResourceLoader*, std::filesystem::path> meshesDataLoaders;
		for (const auto& entry : std::filesystem::directory_iterator(rootDirectory))
		{
			std::filesystem::path filePath = entry.path();
			if (filePath.extension() != ".bin")
			{
				continue;
			}

			ResourceCompilerModule::ResourceType binType = ResourceCompilerModule::GetResourceType(filePath.string().c_str());
			if (binType != ResourceCompilerModule::ResourceType::RESOURCE_TYPE_MESH)
			{
				continue;
			}

			ResourceCompilerModule::ResourceLoader* loader = ResourceCompilerModule::LoadFile(filePath.string().c_str());
			if (loader)
			{
				meshesDataLoaders[loader] = filePath;
			}
		}
		// Collect all static mesh defs (rc)
		std::map<std::string, ResourceCompilerModule::Mesh*> rcAllMeshes;
		std::map<ResourceCompilerModule::Mesh*, ResourceCompilerModule::ResourceLoader*> rcMeshesLoaderLookup;
		for (auto& meshesDataLoader : meshesDataLoaders)
		{
			ResourceCompilerModule::MeshesData* meshesData = meshesDataLoader.first->GetMeshesData();
			if (meshesData)
			{
				std::vector<ResourceCompilerModule::Mesh*> rcMeshes;
				meshesData->GetMeshes(rcMeshes);
				for (auto& rcMesh : rcMeshes)
				{
					if (!rcAllMeshes.contains(rcMesh->GetName()))
					{
						rcAllMeshes[rcMesh->GetName()] = rcMesh;
						rcMeshesLoaderLookup[rcMesh] = meshesDataLoader.first;
					}
				}
			}
		}
		

		// Get meshes parts
		for (auto& tile : m_tiles)
		{
			if (!tile)
			{
				continue;
			}

			// Get all static mesh components on this tile
			const std::vector<StaticMeshComponent*>& allStaticMeshes = tile->GetAllStaticMeshComponents();
			for (auto& staticMeshComp : allStaticMeshes)
			{
				IMesh* mesh = staticMeshComp->GetMesh();
				if (mesh && rcAllMeshes.contains(mesh->GetName()))
				{
					// Find rc mesh ptr
					ResourceCompilerModule::Mesh* rcMesh = rcAllMeshes[mesh->GetName()];
					// Find rc meshes loader ptr
					ResourceCompilerModule::ResourceLoader* meshesLoader = rcMeshesLoaderLookup[rcMesh];
					// Find path
					std::filesystem::path meshesDataPath = meshesDataLoaders[meshesLoader];
					mesh->SetResourceFilePath(meshesDataPath);

					std::vector<ResourceCompilerModule::MeshPart*> rcMeshParts;
					rcMesh->GetMeshParts(rcMeshParts);
					for (auto& rcMeshPart : rcMeshParts)
					{
						ResourceCompilerModule::Material* rcMaterial = rcMeshPart->GetMaterial();
						mesh->AddFileMetadataPart(rcMeshPart->GetDataOffset(), rcMeshPart->GetDataSize(), rcMaterial ? rcMaterial->GetMaterialName() : "");
						if (rcMaterial)
						{
							m_materialManager->CreateMaterial(
								rcMaterial->GetMaterialName(),
								rcMaterial->GetBaseColorTextureName(),
								rcMaterial->GetMetallicTextureName(),
								rcMaterial->GetRoughnessTextureName(),
								rcMaterial->GetNormalTextureName());
						}
					}
				}
			}
		}

		// Release meshes data loaders
		for (auto& meshesDataLoader : meshesDataLoaders)
		{
			if (meshesDataLoader.first)
			{
				delete meshesDataLoader.first;
			}
		}
		meshesDataLoaders.clear();

		// Load all textures
		for (const auto& entry : std::filesystem::directory_iterator(rootDirectory))
		{
			std::filesystem::path filePath = entry.path();
			if (filePath.extension() != ".bin")
			{
				continue;
			}

			ResourceCompilerModule::ResourceType binType = ResourceCompilerModule::GetResourceType(filePath.string().c_str());
			if (binType != ResourceCompilerModule::ResourceType::RESOURCE_TYPE_TEXTURE)
			{
				continue;
			}

			ResourceCompilerModule::ResourceLoader* loader = ResourceCompilerModule::LoadFile(filePath.string().c_str());
			
			ResourceCompilerModule::TexturesData* rcTexturesData = loader->GetTexturesData();
			if (!rcTexturesData)
			{
				continue;
			}

			std::vector<ResourceCompilerModule::Texture*> rcTextures;
			rcTexturesData->GetTextures(rcTextures);
			for (auto& rcTexture : rcTextures)
			{
				if (!rcTexture)
				{
					continue;
				}
			
				m_textureManager->GetOrCreateTexture(
					rcTexture->GetTextureName(),
					filePath,
					rcTexture->GetTextureDataOffset(),
					rcTexture->GetTextureDataSize(),
					rcTexture->GetWidth(),
					rcTexture->GetHeight(),
					rcTexture->GetNumChannels());
			}

			delete loader;
		}

		return true;
	}

	return false;
}

bool World2::StreamInAroundActiveCameraRange(GraphicsContext* graphicsContext, CommandBuilder* cmdBuilder, float radius)
{
	Camera* activeCamera = GetActiveCamera();
	if (!activeCamera)
	{
		return true;
	}

	FVector3 camPos = activeCamera->GetPosition();
	bool updated = false;
	for (auto& tile : m_tiles)
	{
		float xmin = 0.0f, xmax = 0.0f, ymin = 0.0f, ymax = 0.0f;
		tile->GetBBox(xmin, xmax, ymin, ymax);

		float deltaX = camPos.X - max(xmin, min(camPos.X, xmax));
		float deltaY = camPos.Y - max(ymin, min(camPos.Y, xmax));

		// Check if the tile bbox is within the range
		bool withInRange = (deltaX * deltaX + deltaY * deltaY) < (radius * radius);

		if (withInRange)
		{
			if (!tile->IsStreamedIn())
			{
				updated = true;
				tile->StreamInBinary(graphicsContext, cmdBuilder);
			}
		}
		else
		{
			// TODO: if not within range, schedule to stream out
		}
	}

	return false;
}

bool World2::UpdateBuffersForFrame()
{
	Camera* activeCamera = GetActiveCamera();
	if (!activeCamera)
	{
		return false;
	}

	// Update uniform frame constants
	UniformFrameConstants uniformFrameConstants = {};
	DirectX::XMStoreFloat4x4(&uniformFrameConstants.ViewMatrix, DirectX::XMMatrixTranspose(activeCamera->GetViewMatrix()));
	DirectX::XMStoreFloat4x4(&uniformFrameConstants.ProjectionMatrix, DirectX::XMMatrixTranspose(activeCamera->GetProjectionMatrix()));

	XMMATRIX viewProj = DirectX::XMMatrixTranspose(activeCamera->GetProjectionMatrix()) * DirectX::XMMatrixTranspose(activeCamera->GetViewMatrix());

	XMVECTOR detViewProj;
	XMMATRIX invViewProj = DirectX::XMMatrixInverse(&detViewProj, viewProj);

	XMVECTOR detProj;
	XMMATRIX invProj = DirectX::XMMatrixInverse(&detProj, DirectX::XMMatrixTranspose(activeCamera->GetProjectionMatrix()));

	XMVECTOR detView;
	XMMATRIX invView = DirectX::XMMatrixInverse(&detView, DirectX::XMMatrixTranspose(activeCamera->GetViewMatrix()));

	XMVECTOR detVectorView;
	XMMATRIX vectorView = DirectX::XMMatrixInverse(&detVectorView, activeCamera->GetViewMatrix());


	XMVECTOR forwardVector = DirectX::XMVector3Transform(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), invView);

	DirectX::XMStoreFloat4x4(&uniformFrameConstants.ViewProjectionMatrix, viewProj);
	DirectX::XMStoreFloat4x4(&uniformFrameConstants.InvViewProjectionMatrix, invViewProj);
	DirectX::XMStoreFloat4x4(&uniformFrameConstants.InvProjectionMatrix, invProj);
	DirectX::XMStoreFloat4x4(&uniformFrameConstants.InvViewMatrix, invView);
	DirectX::XMStoreFloat4x4(&uniformFrameConstants.VectorViewMatrix, vectorView);


	FVector3 cameraPos = activeCamera->GetPosition();
	uniformFrameConstants.CameraPostion[0] = cameraPos.X;
	uniformFrameConstants.CameraPostion[1] = cameraPos.Y;
	uniformFrameConstants.CameraPostion[2] = cameraPos.Z;

	uniformFrameConstants.ForwardVector[0] = forwardVector.m128_f32[0];
	uniformFrameConstants.ForwardVector[1] = forwardVector.m128_f32[1];
	uniformFrameConstants.ForwardVector[2] = forwardVector.m128_f32[2];
	uniformFrameConstants.ForwardVector[3] = forwardVector.m128_f32[3];

	uniformFrameConstants.PixelStepScale = activeCamera->GetPixelStepScale();
	uniformFrameConstants.RenderTargetHeight = activeCamera->GetWidth();
	uniformFrameConstants.RenderTargetWidth = activeCamera->GetHeight();
	uniformFrameConstants.PixelWidthInNdc = 2.0f / (float)activeCamera->GetWidth();
	uniformFrameConstants.PixelHeightInNdc = 2.0f / (float)activeCamera->GetHeight();

	(*m_uniformFrameConstantBuffer)[0] = uniformFrameConstants;
	bool succ = m_uniformFrameConstantBuffer->UpdateToGPU();

	// Update tile states.
	for (auto& tile : m_tiles)
	{
		if (!tile->IsStreamedIn())
		{
			continue;
		}

		tile->UpdateBuffersForFrame();
	}

	

	m_lightConstantBuffer->UpdateToGPU();

	return succ;
}
void World2::GetActiveTiles(std::vector<Tile*>& outTiles)
{
	outTiles.clear();
	for (auto& tile : m_tiles)
	{
		if (tile->IsStreamedIn())
		{
			outTiles.push_back(tile.get());
		}
	}
}