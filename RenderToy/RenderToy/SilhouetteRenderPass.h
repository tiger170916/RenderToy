#pragma once

#include "Includes.h"
#include "RenderPassBase.h"
#include "ShaderManager.h"

class SilhouetteRenderPass : public RenderPassBase
{
private:
	std::unique_ptr<GraphicsPipelineState> m_depthPipelineState = nullptr;

	std::unique_ptr<GraphicsPipelineState> m_stencilPipelineState = nullptr;

	std::unique_ptr<GraphicsPipelineState> m_silhouettePipelineState = nullptr;

	ComPtr<ID3D12Resource> m_depthStencilBuffer = nullptr;

	DXGI_FORMAT m_depthFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	UINT64 m_dsvId = UINT64_MAX;

	D3D12_VIEWPORT m_viewport;

	D3D12_RECT m_scissorRect;

public:
	SilhouetteRenderPass(GUID passGuid);

	~SilhouetteRenderPass();

	// Interface
	virtual bool Initialize(GraphicsContext* graphicsContext, ShaderManager* shaderManager) override;

	virtual bool PopulateCommands(World* world, GraphicsContext* graphicsContext) override;
};