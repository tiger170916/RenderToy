#include "World.h"
#include "Macros.h"
#include "ResourceCompilerModuleApi.h"

World::World() {}

World::~World() {}

bool World::Initialize(GraphicsContext* graphicsContext)
{
	if (m_initialized)
	{
		return true;
	}

	if (!graphicsContext)
	{
		return false;
	}

	m_uniformFrameConstantBuffer = std::unique_ptr<ConstantBuffer<UniformFrameConstants>>(new ConstantBuffer<UniformFrameConstants>());
	if (!m_uniformFrameConstantBuffer->Initialize(graphicsContext))
	{
		return false;
	}

	// Create light constants
	m_lightConstants = std::unique_ptr<ConstantBuffer<LightConstantsDx>>(new ConstantBuffer<LightConstantsDx>());
	if (!m_lightConstants->Initialize(graphicsContext))
	{
		return false;
	}


	m_initialized = true;
	return true;
}

bool World::InitializeFromBinary(GraphicsContext* graphicsContext, std::filesystem::path binaryPath)
{
	if (m_initialized)
	{
		return true;
	}

	if (!std::filesystem::exists(binaryPath))
	{
		return false;
	}

	try {
		// Iterate over the entries in world directory
		for (const auto& entry : std::filesystem::directory_iterator(binaryPath)) {
			if (entry.path().extension().string() != ".bin")
			{
				continue;
			}

			LoadTileBinaryFile(entry);
		}
	}
	catch (const std::filesystem::filesystem_error) {
		return false;

	}


	m_initialized = true;

	return true;
}

void World::SetActiveCamera(UINT width, UINT height, FVector3 position, FRotator rotator)
{
	if (m_activeCamera)
	{
		m_activeCamera.reset();
	}

	m_activeCamera = std::unique_ptr<Camera>(new Camera(width, height, position, rotator));
}

void World::SpawnStaticMesh(std::shared_ptr<StaticMesh> staticMesh)
{
	m_staticMeshes.push_back(staticMesh);
}

void World::SpawnStaticMeshes(std::vector<std::shared_ptr<StaticMesh>>& staticMeshes)
{
	for (auto mesh : staticMeshes)
	{
		m_staticMeshes.push_back(mesh);
	}
}

bool World::UpdateBuffers()
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

	// Update light constants
	LightConstantsDx lightConstantsDx = {};
	uint32_t lightIdx = 0;
	for (auto& staticMesh : m_staticMeshes)
	{
		for (uint32_t i = 0; i < staticMesh->GetNumInstances(); i++)
		{
			if (lightIdx >= 50)
			{
				break;
			}

			if (staticMesh->HasLightExtension(i))
			{
				LightExtension* lightExt = staticMesh->GetLightExtension(i);
				if (lightExt)
				{
					Transform instanceTransform = {};
					staticMesh->GetInstanceTransform(i, instanceTransform);
					//lightExt->UpdateLightConstants(lightConstantsDx.Lights[lightIdx], instanceTransform.Translation);
					lightIdx += 1;
				}
			}
		}
	}

	(*m_lightConstants)[0] = lightConstantsDx;
	m_lightConstants->UpdateToGPU();

	for (auto& staticMesh : m_staticMeshes)
	{
		succ &= staticMesh->UpdateBuffers();
	}


	return succ;
}

bool World::FrameBegin(float delta)
{
	
	return true;
}

bool World::LoadTileBinaryFile(std::filesystem::path tileFilePath)
{
	/*
	std::fstream file(tileFilePath, std::ios::in | std::ios::binary);

	ResourceCompilerModule::TileHeader tileHeader = {};
	if (!ReadHeader(&file, tileHeader))
	{
		file.close();
		return false;
	}

	Tile* newTile = new Tile(tileFilePath, tileHeader.TileName, tileHeader.BboxMinX, tileHeader.BboxMinY, tileHeader.BboxMaxX, tileHeader.BboxMaxY);
	m_tiles.push_back(std::unique_ptr<Tile>(newTile));

	newTile->LoadTileContentsFromFile();

	file.close();*/

	return true;
}

bool World::GetAdjacentTilesAroundActiveCamera(std::vector<Tile*>& outAdjacentTiles, std::vector<Tile*> outOtherTiles)
{
	if (!m_activeCamera)
	{
		return false;
	}

	outAdjacentTiles.clear();
	outOtherTiles.clear();

	std::vector<FVector3> points;
	const FVector3& cameraPos = m_activeCamera->GetPosition();
	points.push_back(cameraPos);
	points.push_back(cameraPos + FVector3(50.0f, 50.0f, 0));
	points.push_back(cameraPos + FVector3(-50.0f, 50.0f, 0));
	points.push_back(cameraPos + FVector3(50.0f, -50.0f, 0));
	points.push_back(cameraPos + FVector3(-50.0f, -50.0f, 0));


	for (auto& tile : m_tiles)
	{
		for (auto& point : points)
		{
			if (tile->IsPointInTile(point.X, point.Y))
			{
				outAdjacentTiles.push_back(tile.get());
			}
			else
			{
				outOtherTiles.push_back(tile.get());
			}
		}
	}

	return true;
}