#pragma once

#include "RenderPassBase.h"

class LightShaftPrePass : public RenderPassBase
{
private:
	ComPtr<ID3D12Resource> m_pDepthResource = nullptr;

	ComPtr<ID3D12Resource> m_pPositionBuffer = nullptr;

	DXGI_FORMAT m_depthFormat = DXGI_FORMAT_D32_FLOAT;

	DXGI_FORMAT m_positionBufferFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;

	UINT64 m_depthDsvId = UINT64_MAX;

	UINT64 m_positionUavId = UINT64_MAX;

	UINT64 m_positionRtvId = UINT64_MAX;

	D3D12_VIEWPORT m_viewport;

	D3D12_RECT m_scissorRect;

	const float m_bufferClearValue[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

public:
	LightShaftPrePass(GUID passGuid);

	~LightShaftPrePass();

	// Interface
	virtual bool Initialize(GraphicsContext* graphicsContext, ShaderManager* shaderManager) override;

	virtual bool PopulateCommands(World* world, GraphicsContext* graphicsContext) override;

public:
	// public resoure getter
	inline bool PositionBufferBarrierTransition(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES stateAfter) { return ResourceBarrierTransition(m_pPositionBuffer.Get(), commandList, stateAfter); }
	
	inline const UINT64 GetPositionBufferUavId() const { return m_positionUavId; }
};