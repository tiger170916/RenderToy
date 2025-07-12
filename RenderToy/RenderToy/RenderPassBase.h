#pragma once

#include "Includes.h"
#include "World.h"
#include "GraphicsPipelineState.h"
#include "GraphicsContext.h"

class RenderPassBase
{
protected:
	std::unique_ptr<GraphicsPipelineState> m_graphicsPipelineState = nullptr;

public:
	virtual void Frame(std::shared_ptr<World> world, ID3D12GraphicsCommandList* commandList, GraphicsContext* graphicsContext) = 0;
};