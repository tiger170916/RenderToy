#pragma once

#include "RenderPassBase.h"

class LightShaftPrePass : public RenderPassBase
{
private:
	ComPtr<ID3D12Resource> m_pDepthResource = nullptr;

	DXGI_FORMAT m_depthFormat = DXGI_FORMAT_R32_FLOAT;

	UINT64 m_depthUavId = UINT64_MAX;


	D3D12_VIEWPORT m_viewport;

	D3D12_RECT m_scissorRect;

public:
	LightShaftPrePass(GUID passGuid);

	~LightShaftPrePass();

	// Interface
	virtual bool Initialize(GraphicsContext* graphicsContext, ShaderManager* shaderManager) override;

	virtual bool PopulateCommands(World* world, GraphicsContext* graphicsContext) override;
};