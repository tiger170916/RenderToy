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

	// Initialize render passes
	m_earlyZPass = std::unique_ptr<EarlyZPass>(new EarlyZPass());
	if (!m_earlyZPass->Initialize(m_graphicsContext->GetDevice(), m_graphicsContext->GetHwndWidth(), m_graphicsContext->GetHwndHeight()))
	{
		return false;
	}

	m_activeWorld = std::shared_ptr<World>(new World());
	m_activeWorld;

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

	m_earlyZPass->Frame(m_activeWorld);

	m_lastRenderTime = nowInMicroSecs;
}