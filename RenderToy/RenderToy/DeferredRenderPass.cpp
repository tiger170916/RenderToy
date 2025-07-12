#include "DeferredRenderPass.h"

DeferredRenderPass::DeferredRenderPass()
{

}

DeferredRenderPass::~DeferredRenderPass()
{

}

bool DeferredRenderPass::Initialize(GraphicsContext* graphicsContext, ShaderManager* shaderMgr, UINT width, UINT height)
{
	return false;
}

void DeferredRenderPass::Frame(std::shared_ptr<World> world, ID3D12GraphicsCommandList* commandList, GraphicsContext* graphicsContext, PipelineOutputsStruct& outputs)
{

}