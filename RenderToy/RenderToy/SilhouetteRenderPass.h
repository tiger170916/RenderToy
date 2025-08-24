#pragma once

#include "Includes.h"
#include "RenderPassBase.h"
#include "ShaderManager.h"

class SilhouetteRenderPass : public RenderPassBase
{
private:
	std::unique_ptr<GraphicsPipelineState> m_depthPipelineState = nullptr;

	std::unique_ptr<GraphicsPipelineState> m_stencilPipeline = nullptr;

	std::unique_ptr<GraphicsPipelineState> m_silhouettePipelineState = nullptr;

public:
	SilhouetteRenderPass(GUID passGuid);

	~SilhouetteRenderPass();

	// Interface
	virtual bool Initialize(GraphicsContext* graphicsContext, ShaderManager* shaderManager) override;

	virtual bool PopulateCommands(World* world, GraphicsContext* graphicsContext) override;
};