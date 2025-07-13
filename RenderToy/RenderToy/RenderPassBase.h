#pragma once

#include "Includes.h"
#include "World.h"
#include "GraphicsPipelineState.h"
#include "GraphicsContext.h"
#include "ShaderManager.h"
#include "PipelineOutputs.h"

class RenderPassBase
{
protected:
	std::unique_ptr<GraphicsPipelineState> m_graphicsPipelineState = nullptr;

	bool m_initialized = false;

public:
	virtual bool Initialize(GraphicsContext* graphicsContext, ShaderManager* shaderManager, PipelineResourceStates* pipelineResourceStates) = 0;

	virtual void Frame(World* world, ID3D12GraphicsCommandList* commandList, GraphicsContext* graphicsContext, PipelineResourceStates* pipelineResourceStates, PipelineOutputsStruct& outputs) = 0;
};