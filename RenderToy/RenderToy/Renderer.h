#pragma once
#include "Includes.h"
#include "CriticalSection.h"
#include "GraphicsContext.h"
#include "GraphicsPipelineState.h"
#include "CommandQueue.h"
#include "CommandBuilder.h"
#include "ResourceStreamer.h"
#include "TextureManager.h"
#include "Swapchain.h"
#include "World.h"
#include "ShaderManager.h"
#include "RenderGraph.h"
#include "WndProc.h"
#include "InputManager.h"

/// <summary>
/// Renderer singleton class
/// </summary>
class Renderer
{
private:
	std::unique_ptr<GraphicsContext> m_graphicsContext = nullptr;

	bool m_initialized = false;

	bool m_rendering = false;

	bool m_stateUpdating = false;

	HANDLE m_renderThreadHandle = 0;

	HANDLE m_stateUpdateThreadHandle = 0;

	std::unique_ptr<CriticalSection> m_stateUpdateCriticalSection;

	uint64_t m_lastRenderTime = 0;

	uint64_t m_lastStateUpdateTime = 0;

private:
	std::shared_ptr<World> m_activeWorld = nullptr;	// TODO: switch between different scenes

	std::unique_ptr<ShaderManager> m_shaderManager = nullptr;

	std::unique_ptr<RenderGraph> m_mainRenderGraph = nullptr;

	std::unique_ptr<Swapchain> m_swapchain = nullptr;

	std::unique_ptr<ResourceStreamer> m_resourceStreamer = nullptr;

	std::unique_ptr<TextureManager> m_textureManager = nullptr;

	std::unique_ptr<InputManager> m_inputManager = nullptr;

public:
	~Renderer();

	/// <summary>
	/// Get singleton
	/// </summary>
	static Renderer* Get();

	bool Initialize(HWND hwnd);

	bool RenderStart();

	void RenderStop();

	inline HANDLE GetRenderThreadHandle() const { return m_renderThreadHandle; }

private:
	Renderer() {};

	static DWORD WINAPI RenderThreadRoutine(LPVOID lpParameter);

	static DWORD WINAPI StateUpdateThreadRoutine(LPVOID lpParameter);

	void Frame();

	void FrameBegin(float delta);

	void FrameEnd();
};