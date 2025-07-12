#pragma once

#include "Includes.h"
#include "RenderPassBase.h"
#include "ShaderManager.h"

class EarlyZPass : public RenderPassBase
{
private:
	ComPtr<ID3D12Resource> m_depthStencilBuffer = nullptr;

	DXGI_FORMAT m_depthStencilFormat = DXGI_FORMAT_D32_FLOAT;

	UINT64 m_dsvId = UINT64_MAX;

	bool m_initialized = false;

public:
	EarlyZPass();

	~EarlyZPass();

	bool Initialize(GraphicsContext* graphicsContext, ShaderManager* shaderMgr, UINT width, UINT height);

	virtual void Frame(std::shared_ptr<World> world, ID3D12GraphicsCommandList* commandList, GraphicsContext* graphicsContext) override;
};