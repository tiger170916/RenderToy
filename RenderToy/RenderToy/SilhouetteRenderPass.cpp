#include "SilhouetteRenderPass.h"

SilhouetteRenderPass::SilhouetteRenderPass(GUID passGuid)
	: RenderPassBase(passGuid){}

SilhouetteRenderPass::~SilhouetteRenderPass()
{

}

bool SilhouetteRenderPass::Initialize(GraphicsContext* graphicsContext, ShaderManager* shaderManager)
{
	return true;
}

bool SilhouetteRenderPass::PopulateCommands(World* world, GraphicsContext* graphicsContext)
{
	return true;
}