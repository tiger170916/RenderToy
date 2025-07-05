#pragma once

#include "Includes.h"
#include "CommandQueue.h"

class GraphicsContext
{
	friend class Renderer;

private:
	bool m_initialized = false;

	HWND m_hwnd = 0;

	ComPtr<IDXGIFactory2> m_idxgiFactory = nullptr;

	ComPtr<ID3D12Device> m_pDevice = nullptr;

	ComPtr<IDXGISwapChain3> m_swapchain = nullptr;

	ComPtr<ID3D12DescriptorHeap> m_renderTargetViewHeap = nullptr;

	ComPtr<ID3D12Resource> m_swapchainbuffer[3];

	std::unique_ptr<CommandQueue> m_directCommandQueue = nullptr;

	D3D12_VIEWPORT m_viewport;

	D3D12_RECT m_scissorRect;

	UINT m_rtvDescriptorSize = 0;
	UINT m_dsvDescriptorSize = 0;
	UINT m_cbvSrvDescriptorSize = 0;
	UINT m_samplerDescriptorSize = 0;

	UINT m_width = 0;
	UINT m_height = 0;

	UINT m_numFrameBuffers = 2;

	UINT m_currentBackbuffer = 0;

protected:
	GraphicsContext();

	bool Initialize(HWND hwnd);

public:
	inline UINT GetHwndWidth() const { return m_width; }

	inline UINT GetHwndHeight() const { return m_height; }

	inline ID3D12Device* GetDevice() const { return m_pDevice.Get(); }

	~GraphicsContext();
};