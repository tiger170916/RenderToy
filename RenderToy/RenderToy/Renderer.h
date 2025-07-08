#pragma once
#include "Includes.h"
#include "GraphicsContext.h"
#include "GraphicsPipelineState.h"
#include "World.h"
#include "ShaderManager.h"
#include "EarlyZPass.h"

/// <summary>
/// Renderer singleton class
/// </summary>
class Renderer
{
private:
	std::unique_ptr<GraphicsContext> m_graphicsContext = nullptr;

	bool m_initialized = false;

	bool m_rendering = false;

	HANDLE m_renderThreadHandle = 0;

	uint64_t m_lastRenderTime = 0;

private:
	std::shared_ptr<World> m_activeWorld = nullptr;	// TODO: switch between differnt scenes

	// Render passes
	std::unique_ptr<EarlyZPass> m_earlyZPass = nullptr;

	std::unique_ptr<ShaderManager> m_shaderManager = nullptr;

	std::unique_ptr<DescriptorHeapManager> m_descriptorHeapManager = nullptr;

public:
	~Renderer();

	/// <summary>
	/// Get singleton
	/// </summary>
	static Renderer* Get();

	bool Initialize(HWND hwnd);

	bool RenderStart();

	inline HANDLE GetRenderThreadHandle() const { return m_renderThreadHandle; }

private:
	Renderer() {};

	static DWORD WINAPI RenderThreadRoutine(LPVOID lpParameter);

	void Frame();
};