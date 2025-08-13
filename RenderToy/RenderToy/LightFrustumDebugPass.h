#pragma once

#include "RenderPassBase.h"

class LightFrustumDebugPass : public RenderPassBase
{
private:
	ComPtr<ID3D12Resource> m_pDepthResource = nullptr;

	DXGI_FORMAT m_depthFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	UINT64 m_dsvId = UINT64_MAX;

	D3D12_VIEWPORT m_viewport;

	D3D12_RECT m_scissorRect;

public:
	LightFrustumDebugPass(GUID passGuid);

	~LightFrustumDebugPass();

	// Interface
	virtual bool Initialize(GraphicsContext* graphicsContext, ShaderManager* shaderManager) override;

	virtual bool PopulateCommands(World* world, GraphicsContext* graphicsContext) override;
};