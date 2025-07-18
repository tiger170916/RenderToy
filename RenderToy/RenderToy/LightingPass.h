#pragma once
#include "Includes.h"
#include "RenderPassBase.h"
#include "StaticMesh.h"

class LightingPass : public RenderPassBase
{
private:
	ComPtr<ID3D12Resource> m_renderTarget = nullptr;

	UINT64 m_rtvId = UINT64_MAX;

	const DXGI_FORMAT m_renderTargetFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	const float m_renderTargetClearValue[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	std::unique_ptr<StaticMesh> m_rectangleMesh = nullptr;

	D3D12_VIEWPORT m_viewport;

	D3D12_RECT m_scissorRect;

public:
	LightingPass(GUID passGuid);

	~LightingPass();

	// Interface
	virtual bool Initialize(GraphicsContext* graphicsContext, ShaderManager* shaderManager) override;

	virtual bool PopulateCommands(World* world, GraphicsContext* graphicsContext) override;

public:
	// public resource getter
	inline bool RenderTargetBufferBarrierTransition(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES stateAfter) { return ResourceBarrierTransition(m_renderTarget.Get(), commandList, stateAfter); }

	inline const UINT64 GetRenderTargetBufferRtvId() const { return m_rtvId; }
};