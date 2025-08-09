#pragma once

#include "Includes.h"
#include "RenderPassBase.h"
#include "TextureAtlas.h"
#include "ConstantBuffer.h"
#include "LightStructs.h"
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
	std::unique_ptr<TextureAtlas> m_atlas = nullptr;

	DXGI_FORMAT m_depthFormat = DXGI_FORMAT_R32_FLOAT;

	DXGI_FORMAT m_rtDepthFormat = DXGI_FORMAT_D32_FLOAT;

	UINT64 m_depthAtlasUavId = UINT64_MAX;

	const UINT m_depthAtlasBufferSize = 4096;

	const UINT m_l1ShadowMapSize = 1024;

	const UINT m_l2ShadowMapSize = 512;

	std::map<LightExtension*, std::vector<LightMapInfo>> m_currentFrameLights;

	// Currently support 2 levels of shadow map resolutions.
	D3D12_VIEWPORT m_viewports[2];

	D3D12_RECT m_scissorRects[2];

	ComPtr<ID3D12Resource> m_depthStencilBuffers[2] = { nullptr, nullptr };

	UINT64 m_dsvIds[2] = { UINT64_MAX, UINT64_MAX };

	UINT m_maxNumLightSource = 100;

public:
	ShadowPass(GUID passGuid);

	~ShadowPass();

	// Interface
	virtual bool Initialize(GraphicsContext* graphicsContext, ShaderManager* shaderManager) override;

	virtual bool PopulateCommands(World* world, GraphicsContext* graphicsContext) override;

	// 
	inline bool DepthAtlasBarrierTransition(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES stateAfter) { return ResourceBarrierTransition(m_atlas->GetResource(), commandList, stateAfter); }

	inline UINT64 GetDepthAtlasUavId() const { return m_depthAtlasUavId; }

private:
};