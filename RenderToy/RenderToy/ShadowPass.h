#pragma once

#include "Includes.h"
#include "RenderPassBase.h"
#include "Lights/LightExtension.h"

class ShadowPass : public RenderPassBase
{
private:
	struct LightMapInfo
	{
		UINT ResolutionLevel;
		UINT OffsetX;
		UINT OffsetY;
	};

private:
	ComPtr<ID3D12Resource> m_depthAtalasBuffer = nullptr;

	DXGI_FORMAT m_depthFormat = DXGI_FORMAT_R32_FLOAT;

	UINT64 m_depthAtlasUavId = UINT64_MAX;

	const UINT m_depthAtlasBufferSize = 4096;

	const UINT m_l1ShadowMapSize = 128;

	const UINT m_l2ShadowMapSize = 256;

	UINT m_numTiles = 0;

	std::vector<std::vector<bool>> m_occupied;

	std::map<LightExtension*, std::vector<LightMapInfo>> m_currentFrameLights;

	// Currently support 2 levels of shadow map resolutions.
	D3D12_VIEWPORT m_viewports[2];

	D3D12_RECT m_scissorRects[2];

public:
	ShadowPass(GUID passGuid);

	~ShadowPass();

	// Interface
	virtual bool Initialize(GraphicsContext* graphicsContext, ShaderManager* shaderManager) override;

	virtual bool PopulateCommands(World* world, GraphicsContext* graphicsContext) override;
};