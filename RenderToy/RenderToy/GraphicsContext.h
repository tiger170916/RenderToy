#pragma once

#include "Includes.h"
#include "CommandQueue.h"
#include "CommandBuilder.h"
#include "DescriptorHeapManager.h"

class GraphicsContext
{
	friend class Renderer;

private:
	bool m_initialized = false;

	HWND m_hwnd = 0;

	ComPtr<IDXGIFactory2> m_idxgiFactory = nullptr;

	ComPtr<ID3D12Device> m_pDevice = nullptr;

	ComPtr<IDXGISwapChain3> m_swapchain = nullptr;

	ComPtr<ID3D12Resource> m_swapchainBuffers[3];

	std::unique_ptr<CommandQueue> m_swapchainCommandQueue = nullptr;

	std::unique_ptr<CommandBuilder> m_swapchainCommandBuilder = nullptr;

	std::unique_ptr<DescriptorHeapManager> m_descriptorHeapManager = nullptr;

	D3D12_VIEWPORT m_viewport;

	D3D12_RECT m_scissorRect;

	UINT m_rtvDescriptorSize = 0;
	UINT m_dsvDescriptorSize = 0;
	UINT m_cbvSrvDescriptorSize = 0;
	UINT m_samplerDescriptorSize = 0;

	UINT m_width = 0;
	UINT m_height = 0;

	uint64_t m_rtvIds[4] = {};		// Max size of render target is 4

	UINT m_numFrameBuffers = 2;

	UINT m_currentBackbuffer = 0;

	UINT m_adapterNodeMask = 0;

protected:
	GraphicsContext();

	bool Initialize(HWND hwnd);

public:
	inline UINT GetHwndWidth() const { return m_width; }

	inline UINT GetHwndHeight() const { return m_height; }

	inline ID3D12Device* GetDevice() const { return m_pDevice.Get(); }

	inline const UINT GetAdapterNodeMask() const { return m_adapterNodeMask; }

	inline DescriptorHeapManager* GetDescriptorHeapManager() const { return m_descriptorHeapManager.get(); }

	bool CopyToCurrentBackBuffer();

	bool PresentCurrentBackBuffer();


	~GraphicsContext();
};