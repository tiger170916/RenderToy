#include "World2.h"
#include "ResourceCompilerModuleApi.h"

World2::World2(MaterialManager* materialManager, TextureManager2* texManager)
	: m_materialManager(materialManager), m_textureManager(texManager) {
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

void World2::CreateCamera(UINT width, UINT height, FVector3 position, FRotator rotator)
{
	if (m_activeCamera)
	{
		m_activeCamera.reset();
	}

	m_activeCamera = std::unique_ptr<Camera>(new Camera(width, height, position, rotator));
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

			std::fstream file(filePath, std::ios::in | std::ios::binary);
			if (!file.is_open())
			{
				continue;
			}

			ResourceCompilerModule::BinaryHeader header = {};
			if (ReadHeader(&file, header))
			{
				// Only accept tile binary files here

				if (header.BinaryType == ResourceCompilerModule::BinaryType::BINARY_TYPE_TILE)
				{
					std::unique_ptr<Tile> tile = std::make_unique<Tile>(header.TileName, m_materialManager, m_textureManager, header.BboxMinX, header.BboxMinY, header.BboxMaxX, header.BboxMaxY);
					tile->LoadTileContentsFromFile(filePath);
					m_tiles.push_back(std::move(tile));
				}
			}

			file.close();
		}

		// Load all meshes & materials
		for (const auto& entry : std::filesystem::directory_iterator(rootDirectory))
		{
			std::filesystem::path filePath = entry.path();
			if (filePath.extension() != ".bin")
			{
				continue;
			}

			std::fstream file(filePath, std::ios::in | std::ios::binary);
			if (!file.is_open())
			{
				continue;
			}

			ResourceCompilerModule::BinaryHeader header = {};
			if (ReadHeader(&file, header))
			{
				if (header.BinaryType == ResourceCompilerModule::BinaryType::BINARY_TYPE_MESHES)
				{
					ResourceCompilerModule::StaticMeshDefinitionHeader* staticMeshDefinitionHeaders = nullptr;
					ResourceCompilerModule::MeshPartHeader* meshPartHeaders = nullptr;
					ResourceCompilerModule::MaterialHeader* materialHeaders = nullptr;

					uint32_t numStaticMeshDefinitions = 0;
					uint32_t numMeshParts = 0;
					uint32_t numMaterials = 0;
					
					bool succ = GetStaticMeshDefinitions(&file, header, &staticMeshDefinitionHeaders, numStaticMeshDefinitions);
					succ &= GetMeshPartHeaders(&file, header, &meshPartHeaders, numMeshParts);
					succ &= GetMaterialHeaders(&file, header, &materialHeaders, numMaterials);

					if (succ)
					{
						for (auto& tile : m_tiles)
						{
							tile->LoadMeshFromFile(
								filePath,
								staticMeshDefinitionHeaders, meshPartHeaders,
								materialHeaders,
								numStaticMeshDefinitions,
								numMeshParts,
								numMaterials);
						}

						for (uint32_t matIdx = 0; matIdx < numMaterials; matIdx++)
						{
							ResourceCompilerModule::MaterialHeader& matHeader = materialHeaders[matIdx];
							m_materialManager->CreateMaterial(
								matHeader.MaterialName,
								matHeader.BaseColorTextureName,
								matHeader.MetallicTextureName,
								matHeader.RoughnessTextureName,
								matHeader.NormalTextureName);
						}
					}

					if (numStaticMeshDefinitions > 0)
					{
						delete staticMeshDefinitionHeaders;
					}

					if (numMeshParts > 0)
					{
						delete meshPartHeaders;
					}

					if (numMaterials > 0)
					{
						delete materialHeaders;
					}
				}
			}

			file.close();
		}

		// Load all textures
		for (const auto& entry : std::filesystem::directory_iterator(rootDirectory))
		{
			std::filesystem::path filePath = entry.path();
			if (filePath.extension() != ".bin")
			{
				continue;
			}

			std::fstream file(filePath, std::ios::in | std::ios::binary);
			if (!file.is_open())
			{
				continue;
			}

			ResourceCompilerModule::BinaryHeader header = {};
			if (ReadHeader(&file, header))
			{
				if (header.BinaryType == ResourceCompilerModule::BinaryType::BINARY_TYPE_TEXTURES)
				{
					ResourceCompilerModule::TextureHeader* textureHeaders = nullptr;
					uint32_t numTextures = 0;
					if (GetTextureHeaders(&file, header, &textureHeaders, numTextures))
					{
						for (uint32_t texIdx = 0; texIdx < numTextures; texIdx++)
						{
							ResourceCompilerModule::TextureHeader& texHeader = textureHeaders[texIdx];
							m_textureManager->GetOrCreateTexture(
								texHeader.TextureName,
								filePath,
								texHeader.TextureOffset,
								texHeader.TextureDataSize,
								texHeader.TextureWidth,
								texHeader.TextureHeight,
								texHeader.NumChannels);
						}
					}
				}
			}
		}


		return true;
	}

	return false;
}

bool World2::StreamInAroundActiveCameraRange(GraphicsContext* graphicsContext, CommandBuilder* cmdBuilder, float radius)
{
	FVector3 camPos = m_activeCamera->GetPosition();
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

	if (updated)
	{
		// Update tile states.
		for (auto& tile : m_tiles)
		{
			tile->GetAllStaticMeshes(m_activeMeshes);
		}
	}

	return false;
}

void World2::GetActiveStaticMeshes(std::vector<StaticMesh2*>& outMeshes)
{
	outMeshes.clear();

	for (auto& tile : m_tiles)
	{
		if (tile->IsStreamedIn())
		{
			tile->GetAllStaticMeshes(outMeshes);
		}
	}
}

bool World2::UpdateBuffersForFrame()
{
	// Update uniform frame constants
	UniformFrameConstants uniformFrameConstants = {};
	DirectX::XMStoreFloat4x4(&uniformFrameConstants.ViewMatrix, DirectX::XMMatrixTranspose(m_activeCamera->GetViewMatrix()));
	DirectX::XMStoreFloat4x4(&uniformFrameConstants.ProjectionMatrix, DirectX::XMMatrixTranspose(m_activeCamera->GetProjectionMatrix()));

	XMMATRIX viewProj = DirectX::XMMatrixTranspose(m_activeCamera->GetProjectionMatrix()) * DirectX::XMMatrixTranspose(m_activeCamera->GetViewMatrix());

	XMVECTOR detViewProj;
	XMMATRIX invViewProj = DirectX::XMMatrixInverse(&detViewProj, viewProj);

	XMVECTOR detProj;
	XMMATRIX invProj = DirectX::XMMatrixInverse(&detProj, DirectX::XMMatrixTranspose(m_activeCamera->GetProjectionMatrix()));

	XMVECTOR detView;
	XMMATRIX invView = DirectX::XMMatrixInverse(&detView, DirectX::XMMatrixTranspose(m_activeCamera->GetViewMatrix()));

	XMVECTOR detVectorView;
	XMMATRIX vectorView = DirectX::XMMatrixInverse(&detVectorView, m_activeCamera->GetViewMatrix());


	XMVECTOR forwardVector = DirectX::XMVector3Transform(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), invView);

	DirectX::XMStoreFloat4x4(&uniformFrameConstants.ViewProjectionMatrix, viewProj);
	DirectX::XMStoreFloat4x4(&uniformFrameConstants.InvViewProjectionMatrix, invViewProj);
	DirectX::XMStoreFloat4x4(&uniformFrameConstants.InvProjectionMatrix, invProj);
	DirectX::XMStoreFloat4x4(&uniformFrameConstants.InvViewMatrix, invView);
	DirectX::XMStoreFloat4x4(&uniformFrameConstants.VectorViewMatrix, vectorView);


	FVector3 cameraPos = m_activeCamera->GetPosition();
	uniformFrameConstants.CameraPostion[0] = cameraPos.X;
	uniformFrameConstants.CameraPostion[1] = cameraPos.Y;
	uniformFrameConstants.CameraPostion[2] = cameraPos.Z;

	uniformFrameConstants.ForwardVector[0] = forwardVector.m128_f32[0];
	uniformFrameConstants.ForwardVector[1] = forwardVector.m128_f32[1];
	uniformFrameConstants.ForwardVector[2] = forwardVector.m128_f32[2];
	uniformFrameConstants.ForwardVector[3] = forwardVector.m128_f32[3];

	uniformFrameConstants.PixelStepScale = m_activeCamera->GetPixelStepScale();
	uniformFrameConstants.RenderTargetHeight = m_activeCamera->GetWidth();
	uniformFrameConstants.RenderTargetWidth = m_activeCamera->GetHeight();
	uniformFrameConstants.PixelWidthInNdc = 2.0f / (float)m_activeCamera->GetWidth();
	uniformFrameConstants.PixelHeightInNdc = 2.0f / (float)m_activeCamera->GetHeight();

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