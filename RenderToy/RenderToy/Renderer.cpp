#include "Renderer.h"
#include "FbxLoader.h"
#include "Lights/LightFactory.h"
#include "GraphicsUtils.h"

Renderer::~Renderer()
{
}

void Renderer::RenderStop()
{
	if (m_resourceStreamer)
	{
		m_resourceStreamer->StopStreaming();
		m_resourceStreamer.reset();
	}
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

	m_shaderManager = std::make_unique<ShaderManager>();

	m_textureManager = std::unique_ptr<TextureManager>(new TextureManager());

	m_resourceStreamer = std::unique_ptr<ResourceStreamer>(new ResourceStreamer());
	if (!m_resourceStreamer->StartStreaming(m_graphicsContext.get()))
	{
		return false;
	}

	m_swapchain = std::unique_ptr<Swapchain>(new Swapchain());
	if (!m_swapchain->Initialize(m_graphicsContext.get())) 
	{
		return false;
	}

	m_mainRenderGraph = std::unique_ptr<RenderGraph>(new RenderGraph("C:\\ZemingGit\\RenderToy\\RenderToy\\RenderGraphs\\MainRenderGraph.json"));
	if (!m_mainRenderGraph->Initialize(m_graphicsContext.get(), m_shaderManager.get()))
	{
		return false;
	}


	m_activeWorld = std::shared_ptr<World>(new World());
	m_activeWorld->SetActiveCamera(m_graphicsContext->GetHwndWidth(), m_graphicsContext->GetHwndHeight(), FVector3(0, 3.0, -6.5), FRotator::Zero());
	m_activeWorld->Initialize(m_graphicsContext.get());

	// Test world
	std::vector<std::shared_ptr<StaticMesh>> meshes;
	std::vector<std::shared_ptr<StaticMesh>> lightBulbMesh;

	//FbxLoader* fbxLoader = new FbxLoader("C:\\Users\\erlie\\Desktop\\Assets\\abandoned-house\\source\\abandonhouse.fbx");
	FbxLoader* fbxLoader = new FbxLoader("C:\\Users\\erlie\\Desktop\\Assets\\village-house\\source\\House_Low.fbx");
	FbxLoader* lightBulbLoader = new FbxLoader("C:\\Users\\erlie\\Desktop\\Assets\\cc0-light-bulb\\source\\LightBulb.fbx", 5);
	fbxLoader->Load(meshes);
	lightBulbLoader->Load(lightBulbMesh);
	for (auto& mesh : meshes)
	{
		Transform transform = Transform::Identity();
		transform.Rotation.Pitch = -DirectX::XM_PI * 0.5f;
		mesh->AddInstance(transform);
		mesh->EnablePass(PassType::EARLY_Z_PASS);
		mesh->EnablePass(PassType::GEOMETRY_PASS);
		mesh->EnablePass(PassType::SHADOW_PASS);

		mesh->BuildResource(m_graphicsContext.get(), m_textureManager.get());
		mesh->QueueStreamingTasks(m_resourceStreamer.get(), 0);
	}


	for (int i = 0; i < lightBulbMesh.size(); i++)
	{
		Transform lightBulbTransform = Transform::Identity();
		lightBulbTransform.Translation = FVector3(0.0f, 1.0f, -8.5f);
		lightBulbTransform.Rotation.Pitch = DirectX::XM_PI * 0.5f;
		lightBulbMesh[i]->AddInstance(lightBulbTransform);
		lightBulbMesh[i]->EnablePass(PassType::EARLY_Z_PASS);
		lightBulbMesh[i]->EnablePass(PassType::GEOMETRY_PASS);

		// Create a light extension, and attach to the light bulb model
		LightFactory::Get()->SpawnSpotLight(lightBulbMesh[i].get(), 100.0, FVector3(0.0f, 0.0f, 0.2f), FVector3(30.0f, 30.0f, 30.0f), FRotator(0.0f, 0.0f, 0.0f), 1.0f, 0.5f);

		lightBulbMesh[i]->BuildResource(m_graphicsContext.get(), m_textureManager.get());
		lightBulbMesh[i]->QueueStreamingTasks(m_resourceStreamer.get(), 0);
	}

	m_activeWorld->SpawnStaticMeshes(meshes);
	m_activeWorld->SpawnStaticMeshes(lightBulbMesh);

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

		Sleep(10);
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

	FrameBegin(delta);

	m_mainRenderGraph->PopulateCommandLists(m_activeWorld.get(), m_graphicsContext.get());

	FrameEnd();

	m_lastRenderTime = nowInMicroSecs;
}

void Renderer::FrameBegin(float delta)
{
	m_activeWorld->FrameBegin(delta);
}

void Renderer::FrameEnd()
{
	m_mainRenderGraph->ExecuteCommands();

	// Wait for all gpu work done...
	m_mainRenderGraph->WaitForRenderFinalOutputDone();
	m_swapchain->CopyToBackbuffer(m_graphicsContext.get(), m_mainRenderGraph->GetFinalRenderOutputResource());
	m_swapchain->Present();
}