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

	UINT64 m_srvId = UINT64_MAX;

	D3D12_VIEWPORT m_viewport;

	D3D12_RECT m_scissorRect;

public:
	EarlyZPass(GUID passGuid);

	~EarlyZPass();

	// Interface
	virtual bool Initialize(GraphicsContext* graphicsContext, ShaderManager* shaderManager) override;

	virtual bool PopulateCommands(World* world, GraphicsContext* graphicsContext) override;

public:
	// public resource getters
	inline bool DepthBufferBarrierTransition(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES stateAfter) { return ResourceBarrierTransition(m_depthStencilBuffer.Get(), commandList, stateAfter); }

	inline const UINT64 GetDepthBufferDsvId() const { return m_dsvId; }

	inline const UINT64 GetDepthBufferSrvId() const { return m_dsvId; }
};