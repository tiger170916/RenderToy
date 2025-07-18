#pragma once

#include "Includes.h"
#include "RenderPassBase.h"

class GeometryPass : public RenderPassBase
{
private:
	ComPtr<ID3D12Resource> m_depthStencilBuffer = nullptr;

	ComPtr<ID3D12Resource> m_diffuseBuffer = nullptr;

	const DXGI_FORMAT m_depthStencilFormat = DXGI_FORMAT_D32_FLOAT;

	const DXGI_FORMAT m_diffuseRenderTargetFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	const float m_diffuseBufferClearValue[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	D3D12_VIEWPORT m_viewport;

	D3D12_RECT m_scissorRect;

	UINT64 m_dsvId = UINT64_MAX;

	UINT64 m_diffuseRtvId = UINT64_MAX;

	UINT64 m_diffuseSrvId = UINT64_MAX;

public:
	GeometryPass(GUID passGuid);

	~GeometryPass();

	// Interface
	virtual bool Initialize(GraphicsContext* graphicsContext, ShaderManager* shaderManager) override;

	virtual bool PopulateCommands(World* world, GraphicsContext* graphicsContext) override;

public:
	// public resoure getter
	inline bool DiffuseBufferBarrierTransition(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES stateAfter) { return ResourceBarrierTransition(m_diffuseBuffer.Get(), commandList, stateAfter); }

	inline const UINT64 GetDiffuseBufferRtvId() const { return m_diffuseRtvId; }

	inline const UINT64 GetDiffuseBufferSrvId() const { return m_diffuseSrvId; }
};