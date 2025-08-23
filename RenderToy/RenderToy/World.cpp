#include "World.h"
#include "Macros.h"

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

bool World::FrameBegin(float delta)
{
	m_activeCamera->Frame(delta);
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
	XMVECTOR forwardVector = DirectX::XMVector3Transform(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), invView);

	DirectX::XMStoreFloat4x4(&uniformFrameConstants.ViewProjectionMatrix, viewProj);
	DirectX::XMStoreFloat4x4(&uniformFrameConstants.InvViewProjectionMatrix, invViewProj);
	DirectX::XMStoreFloat4x4(&uniformFrameConstants.InvProjectionMatrix, invProj);
	DirectX::XMStoreFloat4x4(&uniformFrameConstants.InvViewMatrix, invView);


	FVector3 cameraPos = m_activeCamera->GetPosition();
	uniformFrameConstants.CameraPostion[0] = cameraPos.X;
	uniformFrameConstants.CameraPostion[1] = cameraPos.Y;
	uniformFrameConstants.CameraPostion[2] = cameraPos.Z;

	uniformFrameConstants.ForwardVector[0] = forwardVector.m128_f32[0];
	uniformFrameConstants.ForwardVector[1] = forwardVector.m128_f32[1];
	uniformFrameConstants.ForwardVector[2] = forwardVector.m128_f32[2];
	uniformFrameConstants.ForwardVector[3] = forwardVector.m128_f32[3];

	uniformFrameConstants.PixelStepScale = m_activeCamera->GetPixelStepScale();

	(*m_uniformFrameConstantBuffer)[0] = uniformFrameConstants;
	m_uniformFrameConstantBuffer->UpdateToGPU();

	return true;
}