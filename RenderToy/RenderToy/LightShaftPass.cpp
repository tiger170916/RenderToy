#include "LightShaftPass.h"

LightShaftPass::LightShaftPass(GUID passGuid)
	: RenderPassBase(passGuid)
{

}

LightShaftPass::~LightShaftPass()
{

}

bool LightShaftPass::Initialize(GraphicsContext* graphicsContext, ShaderManager* shaderManager)
{
	if (m_initialized)
	{
		return true;
	}

	if (!graphicsContext || !shaderManager)
	{
		return false;
	}

	if (!PassBase::Initialize(graphicsContext, shaderManager))
	{
		return false;
	}

	return true;
}

bool LightShaftPass::PopulateCommands(World* world, GraphicsContext* graphicsContext)
{
	if (world == nullptr || graphicsContext == nullptr)
	{
		return false;
	}

	PassBase::PopulateCommands(world, graphicsContext);



	ID3D12GraphicsCommandList* commandList = m_commandBuilder->GetCommandList();
	commandList->Close();

	return true;
}