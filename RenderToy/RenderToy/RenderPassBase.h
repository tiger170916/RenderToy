#pragma once

#include "Includes.h"
#include "PassBase.h"
#include "World.h"
#include "GraphicsPipelineState.h"
#include "GraphicsContext.h"
#include "ShaderManager.h"

class RenderPassBase : public PassBase
{
protected:
	std::unique_ptr<GraphicsPipelineState> m_graphicsPipelineState = nullptr;

public:
	RenderPassBase(GUID passGuid);

	~RenderPassBase();
};