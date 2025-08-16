#pragma once

#include "Includes.h"
#include "RenderPassBase.h"
#include "FullScreenQuad.h"

class LightShaftPass : public RenderPassBase
{
public:
	LightShaftPass(GUID passGuid);

	~LightShaftPass();

	// Interface
	virtual bool Initialize(GraphicsContext* graphicsContext, ShaderManager* shaderManager) override;

	virtual bool PopulateCommands(World* world, GraphicsContext* graphicsContext) override;

};