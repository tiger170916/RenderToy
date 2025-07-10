#include "Renderer.h"
#include "FbxLoader.h"

Renderer::~Renderer()
{

}

Renderer* Renderer::Get()
{
	static std::unique_ptr<Renderer> singleton = std::unique_ptr<Renderer>(new Renderer());

	return singleton.get();
}

bool Renderer::Initialize(HWND hwnd)
{
	if (m_initialized)
	{
		return true;
	}

	// Initialize graphics device
	m_graphicsContext = std::unique_ptr<GraphicsContext>(new GraphicsContext());

	if (!m_graphicsContext->Initialize(hwnd))
	{
		return false;
	}

	m_mainCommandQueue = std::unique_ptr<CommandQueue>(new CommandQueue(D3D12_COMMAND_QUEUE_FLAG_NONE, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_QUEUE_PRIORITY_NORMAL, m_graphicsContext->GetAdapterNodeMask()));
	if (!m_mainCommandQueue->Initialize(m_graphicsContext->GetDevice()))
	{
		return false;
	}

	m_mainCommandBuilder = std::unique_ptr<CommandBuilder>(new CommandBuilder(D3D12_COMMAND_LIST_TYPE_DIRECT));
	if (!m_mainCommandBuilder->Initialize(m_graphicsContext->GetDevice()))
	{
		return false;
	}

	m_shaderManager = std::make_unique<ShaderManager>();

	// Initialize render passes
	m_earlyZPass = std::unique_ptr<EarlyZPass>(new EarlyZPass());
	if (!m_earlyZPass->Initialize(m_graphicsContext->GetDevice(), m_graphicsContext->GetAdapterNodeMask(), m_shaderManager.get(), m_graphicsContext->GetHwndWidth(), m_graphicsContext->GetHwndHeight()))
	{
		return false;
	}

	m_activeWorld = std::shared_ptr<World>(new World());
	m_activeWorld->Initialize(m_graphicsContext->GetDevice(), m_graphicsContext->GetDescriptorHeapManager());

	// Test world
	std::vector<std::shared_ptr<StaticMesh>> meshes;
	FbxLoader* fbxLoader = new FbxLoader("C:\\Users\\erlie\\Desktop\\Assets\\medieval-house-2\\source\\House2\\House2.fbx");
	fbxLoader->Load(meshes);
	for (auto& mesh : meshes)
	{
		mesh->AddInstance(Transform::Identity());
		mesh->EnableRenderPass(RenderPass::EARLY_Z_PASS);
		mesh->EnableRenderPass(RenderPass::DEFFERED_RENDER_PASS);
		mesh->EnableRenderPass(RenderPass::LIGHTING_PASS);

		mesh->BuildResource(m_graphicsContext->GetDevice(), m_graphicsContext->GetDescriptorHeapManager());
	}

	m_activeWorld->SpawnStaticMeshes(meshes);

	m_initialized = true;
	return true;
}

bool Renderer::RenderStart()
{
	m_renderThreadHandle = CreateThread(NULL, 0, RenderThreadRoutine, this, NULL, NULL);
	
	return true;
}

DWORD WINAPI Renderer::RenderThreadRoutine(LPVOID lpParameter)
{
	if (lpParameter == nullptr)
	{
		return false;
	}

	Renderer* renderer = (Renderer*)lpParameter;

	renderer->m_rendering = true;
	while (renderer->m_rendering)
	{
		renderer->Frame();
	}

	return true;
}

void Renderer::Frame()
{
	// Get the count in microseconds
	auto now = std::chrono::high_resolution_clock::now();
	auto duration_since_epoch = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch());
	uint64_t nowInMicroSecs = duration_since_epoch.count();

	float delta = 0.0f;
	if (m_lastRenderTime > 0)
	{
		// Get delta in unit of second
		delta = (float)(nowInMicroSecs - m_lastRenderTime) / 1000000.0f;
	}

	m_mainCommandBuilder->Reset();
	ID3D12GraphicsCommandList* commandList = m_mainCommandBuilder->GetCommandList();

	Camera* activeWorldCamera = m_activeWorld->GetActiveCamera();
	UniformFrameConstants uniformFrameConstants = {};
	DirectX::XMStoreFloat4x4(&uniformFrameConstants.ViewMatrix, DirectX::XMMatrixTranspose(activeWorldCamera->GetViewMatrix()));
	DirectX::XMStoreFloat4x4(&uniformFrameConstants.ProjectionMatrix, DirectX::XMMatrixTranspose(activeWorldCamera->GetProjectionMatrix()));
	ConstantBuffer<UniformFrameConstants>* activeUniformFrameConstantBuffer = m_activeWorld->GetUniformFrameConstantBuffer();
	(*activeUniformFrameConstantBuffer)[0] = uniformFrameConstants;
	activeUniformFrameConstantBuffer->UpdateToGPU();

	D3D12_GPU_DESCRIPTOR_HANDLE uniformFrameGpuHandle = {};
	activeUniformFrameConstantBuffer->BindConstantBufferViewToPipeline(m_graphicsContext->GetDescriptorHeapManager(), uniformFrameGpuHandle);
	commandList->SetGraphicsRootDescriptorTable(0, uniformFrameGpuHandle);

	m_earlyZPass->Frame(m_activeWorld, commandList);

	m_lastRenderTime = nowInMicroSecs;
}