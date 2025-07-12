#include "GeometryPass.h"

GeometryPass::GeometryPass()
{

}

GeometryPass::~GeometryPass()
{

}

bool GeometryPass::Initialize(GraphicsContext* graphicsContext, ShaderManager* shaderMgr, UINT width, UINT height)
{
	return false;
}

void GeometryPass::Frame(std::shared_ptr<World> world, ID3D12GraphicsCommandList* commandList, GraphicsContext* graphicsContext, PipelineOutputsStruct& outputs)
{

}