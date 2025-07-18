#include "Renderer.h"
#include "FbxLoader.h"
#include "GraphicsUtils.h"

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

	m_shaderManager = std::make_unique<ShaderManager>();

	m_mainRenderGraph = std::unique_ptr<RenderGraph>(new RenderGraph("myMainRenderGraph.json"));
	if (!m_mainRenderGraph->Initialize(m_graphicsContext.get(), m_shaderManager.get()))
	{
		return false;
	}


	m_activeWorld = std::shared_ptr<World>(new World());
	m_activeWorld->SetActiveCamera(m_graphicsContext->GetHwndWidth(), m_graphicsContext->GetHwndHeight(), FVector3(0, 0, -20), FRotator::Zero());
	m_activeWorld->Initialize(m_graphicsContext.get());

	// Test world
	std::vector<std::shared_ptr<StaticMesh>> meshes;
	FbxLoader* fbxLoader = new FbxLoader("myFbxPass");
	fbxLoader->Load(meshes);
	for (auto& mesh : meshes)
	{
		mesh->AddInstance(Transform::Identity());
		mesh->EnablePass(PassType::EARLY_Z_PASS);
		mesh->EnablePass(PassType::GEOMETRY_PASS);

		mesh->BuildResource(m_graphicsContext.get());
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

		Sleep(50);
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

	FrameBegin();

	m_mainRenderGraph->PopulateCommandLists(m_activeWorld.get(), m_graphicsContext.get());

	FrameEnd();

	m_lastRenderTime = nowInMicroSecs;
}

void Renderer::FrameBegin()
{
	m_activeWorld->FrameBegin();
}

void Renderer::FrameEnd()
{
	m_mainRenderGraph->ExecuteCommands();

	m_graphicsContext->PresentCurrentBackBuffer();
}