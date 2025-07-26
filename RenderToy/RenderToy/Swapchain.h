#pragma once

#include "Includes.h"
#include "CommandBuilder.h"
#include "CommandQueue.h"
#include "GraphicsContext.h"

class Swapchain
{
private:
	ComPtr<IDXGISwapChain3> m_swapchain = nullptr;

	std::unique_ptr<CommandQueue> m_swapchainCommandQueue = nullptr;

	std::unique_ptr<CommandBuilder> m_swapchainCommandBuilder = nullptr;

	ComPtr<ID3D12Resource> m_swapchainBuffers[4];

	uint64_t m_rtvIds[4] = {};

	UINT m_numFrameBuffers = 2;

	UINT m_currentBackbuffer = 0;

	const float m_rtClearColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f };

	bool m_initialized = false;

public:
	Swapchain();

	~Swapchain();

	bool Initialize(GraphicsContext* graphicsContext);

	bool CopyToBackbuffer(GraphicsContext* graphicsContext, ID3D12Resource* resource);

	bool Present();
};