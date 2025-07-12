#pragma once

#include "Includes.h"
#include "RenderPassBase.h"

class DeferredRenderPass : public RenderPassBase
{
private:
	D3D12_VIEWPORT m_viewport;

	D3D12_RECT m_scissorRect;

	bool m_initialized = false;

public:
	DeferredRenderPass();

	~DeferredRenderPass();

	bool Initialize(GraphicsContext* graphicsContext, ShaderManager* shaderMgr, UINT width, UINT height);

	virtual void Frame(std::shared_ptr<World> world, ID3D12GraphicsCommandList* commandList, GraphicsContext* graphicsContext, PipelineOutputsStruct& outputs) override;
};