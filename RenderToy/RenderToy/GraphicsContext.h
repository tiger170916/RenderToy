#pragma once

#include "Includes.h"

class GraphicsContext
{
	friend class Renderer;

private:
	bool m_initialized = false;

	ComPtr<IDXGIFactory2> m_idxgiFactory = nullptr;

	ComPtr<ID3D12Device> m_pDevice = nullptr;

protected:
	GraphicsContext();

	bool Initialize();

public:
	~GraphicsContext();
};