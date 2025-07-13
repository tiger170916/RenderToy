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
	EarlyZPass();

	~EarlyZPass();

	virtual bool Initialize(GraphicsContext* graphicsContext, ShaderManager* shaderManager, PipelineResourceStates* pipelineResourceStates) override;

	virtual void Frame(World* world, ID3D12GraphicsCommandList* commandList, GraphicsContext* graphicsContext, PipelineResourceStates* pipelineResourceStates, PipelineOutputsStruct& outputs) override;
};