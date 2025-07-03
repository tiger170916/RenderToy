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

	std::unique_ptr<CommandQueue> m_directCommandQueue = nullptr;

	ComPtr<ID3D12Device> m_pDevice = nullptr;

	UINT32 m_numFrameBuffers = 2;

protected:
	GraphicsContext();

	bool Initialize(HWND hwnd);

public:
	~GraphicsContext();
};