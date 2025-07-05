#pragma once

#include "Includes.h"
#include "RenderPassBase.h"

class EarlyZPass : public RenderPassBase
{
private:
	ComPtr<ID3D12DescriptorHeap> m_depthStencilViewHeap = nullptr;

	ComPtr<ID3D12Resource> m_depthStencilBuffer = nullptr;

	DXGI_FORMAT m_depthStencilFormat = DXGI_FORMAT_D32_FLOAT;

	bool m_initialized = false;

public:
	EarlyZPass();

	~EarlyZPass();

	bool Initialize(ID3D12Device* pDevice, UINT width, UINT height);

	virtual void Frame(std::shared_ptr<World> world) override;
};