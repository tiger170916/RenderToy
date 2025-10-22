#pragma once

#include "Includes.h"
#include "RenderPassBase.h"
#include "FullScreenQuad.h"

class LightShaftPass : public RenderPassBase
{
private:
	D3D12_VIEWPORT m_viewport = {};

	D3D12_RECT m_scissorRect = {};

public:
	LightShaftPass(GUID passGuid);

	~LightShaftPass();

	// Interface
	virtual bool Initialize(GraphicsContext* graphicsContext, ShaderManager* shaderManager) override;

	virtual bool PopulateCommands(World* world, GraphicsContext* graphicsContext) override;

	virtual bool PopulateCommands(World2* world, MaterialManager* materialManager, TextureManager2* textureManager, GraphicsContext* graphicsContext) override;

};