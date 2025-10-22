#include "Renderer.h"
#include "FbxLoader.h"
#include "MeshFactory.h"
#include "FullscreenQuad.h"
#include "Lights/LightFactory.h"
#include "GraphicsUtils.h"
#include "Utils.h"


Renderer::~Renderer()
{
}

void Renderer::RenderStop()
{
	if (m_resourceStreamer)
	{
		m_resourceStreamer->StopStreaming();
		m_resourceStreamer.reset();
		m_stateUpdating = false;
		m_rendering = false;
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

	m_stateUpdateCriticalSection = std::unique_ptr<CriticalSection>(new CriticalSection());

	// Initialize graphics device
	m_graphicsContext = std::unique_ptr<GraphicsContext>(new GraphicsContext());

	if (!m_graphicsContext->Initialize(hwnd))
	{
		return false;
	}

	if (!FullScreenQuad::Get()->Initialize(m_graphicsContext.get()))
	{
		return false;
	}

	m_shaderManager = std::make_unique<ShaderManager>();
	m_materialManager = std::make_unique<MaterialManager>();

	m_textureManager = std::unique_ptr<TextureManager>(new TextureManager());
	m_textureManager2 = std::unique_ptr<TextureManager2>(new TextureManager2());

	//m_resourceStreamer = std::unique_ptr<ResourceStreamer>(new ResourceStreamer());
	//if (!m_resourceStreamer->StartStreaming(m_graphicsContext.get()))
	//{
	//	return false;
	//}

	m_swapchain = std::unique_ptr<Swapchain>(new Swapchain());
	if (!m_swapchain->Initialize(m_graphicsContext.get())) 
	{
		return false;
	}

	WndProc::Get()->Initialize(hwnd);

	m_inputManager = std::unique_ptr<InputManager>(new InputManager());
	if (!m_inputManager->Initialize(hwnd))
	{
		return false;
	}

	m_mainRenderGraph = std::unique_ptr<RenderGraph>(new RenderGraph("C:\\ZemingGit\\RenderToy\\RenderToy\\RenderGraphs\\MainRenderGraph.json"));
	if (!m_mainRenderGraph->Initialize(m_graphicsContext.get(), m_shaderManager.get()))
	{
		return false;
	}

	m_streamingEngine = std::unique_ptr<StreamingEngine>(new StreamingEngine());
	if (!m_streamingEngine->Initialize(m_graphicsContext.get(), m_textureManager2.get()))
	{
		return false;
	}

	// Load initial world
	std::filesystem::path initialWorldBinaryPath = Utils::GetWorkingDirectory();
	initialWorldBinaryPath.append("InitialWorld");
	m_activeWorld = std::make_shared<World2>(m_materialManager.get(), m_textureManager2.get());
	
	if (!m_activeWorld->Initialize(m_graphicsContext.get()))
	{
		return false;
	}

	if (!m_activeWorld->LoadFromBinary(initialWorldBinaryPath))
	{
		return false;
	}

	m_mainRenderGraphFence = std::make_unique<D3DFence>();
	if (!m_mainRenderGraphFence->Initialize(m_graphicsContext->GetDevice()))
	{
		return false;
	}

	FRotator initRotation = {};
	//initRotation.Yaw = 3.14;
	m_activeWorld->CreateCamera(m_graphicsContext->GetHwndWidth(), m_graphicsContext->GetHwndHeight(), FVector3(0, 0.0, -25.0), initRotation);

	m_streamingEngine->StartStreaming(m_activeWorld.get());

	m_initialized = true;
	return true;
}

bool Renderer::RenderStart()
{
	m_renderThreadHandle = CreateThread(NULL, 0, RenderThreadRoutine, this, NULL, NULL);
	m_stateUpdateThreadHandle = CreateThread(NULL, 0, StateUpdateThreadRoutine, this, NULL, NULL);
	
	return true;
}

DWORD WINAPI Renderer::StateUpdateThreadRoutine(LPVOID lpParameter)
{
	if (lpParameter == nullptr)
	{
		return 1;
	}

	Renderer* renderer = (Renderer*)lpParameter;
	renderer->m_stateUpdating = true;

	while (renderer->m_stateUpdating)
	{
		uint64_t nowInMicroSecs = Utils::GetCurrentTimeInMicroSec();
		float delta = 0.0f;
		if (renderer->m_lastStateUpdateTime > 0)
		{
			// Get delta in unit of second
			delta = (float)(nowInMicroSecs - renderer->m_lastStateUpdateTime) / 1000000.0f;
		}

		// Update input
		if (renderer->m_inputManager)
		{
			renderer->m_stateUpdateCriticalSection->EnterCriticalSection();

			renderer->m_inputManager->Update(delta);

			renderer->m_stateUpdateCriticalSection->ExitCriticalSection();
		}

		renderer->m_lastStateUpdateTime = nowInMicroSecs;
		Sleep(1);
	}

	return 0;
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
	uint64_t nowInMicroSecs = Utils::GetCurrentTimeInMicroSec();

	float delta = 0.0f;
	if (m_lastRenderTime > 0)
	{
		// Get delta in unit of second
		delta = (float)(nowInMicroSecs - m_lastRenderTime) / 1000000.0f;
	}

	FrameBegin(delta);

	m_mainRenderGraph->PopulateCommandLists(m_activeWorld.get(), m_materialManager.get(), m_textureManager2.get(), m_graphicsContext.get());

	FrameEnd();

	m_lastRenderTime = nowInMicroSecs;
}

void Renderer::FrameBegin(float delta)
{
	m_stateUpdateCriticalSection->EnterCriticalSection();

	//m_mainRenderGraph->UpdateBuffers(m_activeWorld.get());
	m_activeWorld->UpdateBuffersForFrame();

	m_stateUpdateCriticalSection->ExitCriticalSection();

	m_activeWorld->UpdateBuffersForFrame();
}

void Renderer::FrameEnd()
{
	m_mainRenderGraph->ExecuteCommands();

	// Wait for all gpu work done...
	m_mainRenderGraph->WaitForRenderFinalOutputDone();

	m_swapchain->CopyToBackbuffer(m_graphicsContext.get(), m_mainRenderGraph->GetFinalRenderOutputResource());
	m_swapchain->Present();
}