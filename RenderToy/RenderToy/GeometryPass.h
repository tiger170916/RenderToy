#pragma once

#include "Includes.h"
#include "RenderPassBase.h"

class GeometryPass : public RenderPassBase
{
private:
	ComPtr<ID3D12Resource> m_depthStencilBuffer = nullptr;

	ComPtr<ID3D12Resource> m_diffuseBuffer = nullptr;

	ComPtr<ID3D12Resource> m_metallicRoughnessBuffer = nullptr;

	ComPtr<ID3D12Resource> m_normalBuffer = nullptr;

	ComPtr<ID3D12Resource> m_worldPosBuffer = nullptr;

	ComPtr<ID3D12Resource> m_emissionBuffer = nullptr;

	const DXGI_FORMAT m_depthStencilFormat = DXGI_FORMAT_D32_FLOAT;

	const DXGI_FORMAT m_diffuseRenderTargetFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	const DXGI_FORMAT m_metallicRoughnessRenderTargetFormat = DXGI_FORMAT_R8G8_UNORM;

	const DXGI_FORMAT m_normalRenderTargetFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;

	const DXGI_FORMAT m_worldPosRenderTargetFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;

	const DXGI_FORMAT m_emissionRenderTargetFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;

	const float m_bufferClearValue[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	D3D12_VIEWPORT m_viewport;

	D3D12_RECT m_scissorRect;

	UINT64 m_dsvId = UINT64_MAX;


	UINT64 m_diffuseRtvId = UINT64_MAX;

	UINT64 m_metallicRoughnessRtvId = UINT64_MAX;

	UINT64 m_normalRtvId = UINT64_MAX;

	UINT64 m_worldPosRtvId = UINT64_MAX;

	UINT64 m_diffuseSrvId = UINT64_MAX;

	UINT64 m_metallicRoughnessSrvId = UINT64_MAX;

	UINT64 m_normalSrvId = UINT64_MAX;

	UINT64 m_worldPosSrvId = UINT64_MAX;

	UINT64 m_emissionRtvId = UINT64_MAX;

	UINT64 m_emissionSrvId = UINT64_MAX;

	UINT64 m_worldPosUavId = UINT64_MAX;

public:
	GeometryPass(GUID passGuid);

	~GeometryPass();

	// Interface
	virtual bool Initialize(GraphicsContext* graphicsContext, ShaderManager* shaderManager) override;

	virtual bool PopulateCommands(World* world, GraphicsContext* graphicsContext) override;

public:
	// public resoure getter
	inline bool DiffuseBufferBarrierTransition(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES stateAfter) { return ResourceBarrierTransition(m_diffuseBuffer.Get(), commandList, stateAfter); }
	inline bool MetallicRoughnessBufferBarrierTransition(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES stateAfter) { return ResourceBarrierTransition(m_metallicRoughnessBuffer.Get(), commandList, stateAfter); }
	inline bool NormalBufferBarrierTransition(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES stateAfter) { return ResourceBarrierTransition(m_normalBuffer.Get(), commandList, stateAfter); }
	inline bool WorldPosBufferBarrierTransition(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES stateAfter) { return ResourceBarrierTransition(m_worldPosBuffer.Get(), commandList, stateAfter); }
	inline bool WorldEmissionBufferBarrierTransition(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES stateAfter) { return ResourceBarrierTransition(m_emissionBuffer.Get(), commandList, stateAfter); }


	inline const UINT64 GetDiffuseBufferRtvId() const { return m_diffuseRtvId; }
	inline const UINT64 GetMetallicRoughnessBufferRtvId() const { return m_metallicRoughnessRtvId; }
	inline const UINT64 GetNormalBufferRtvId() const { return m_normalRtvId; }
	inline const UINT64 GetWorldPosBufferRtvId() const { return m_worldPosRtvId; }
	inline const UINT64 GetEmissionBufferRtvId() const { return m_emissionRtvId; }



	inline const UINT64 GetDiffuseBufferSrvId() const { return m_diffuseSrvId; }
	inline const UINT64 GetMetallicRoughnessBufferSrvId() const { return m_metallicRoughnessSrvId; }
	inline const UINT64 GetNormalBufferSrvId() const { return m_normalSrvId; }
	inline const UINT64 GetWorldPosBufferSrvId() const { return m_worldPosSrvId; }
	inline const UINT64 GetEmissionBufferSrvId() const { return m_emissionSrvId; }



	inline const UINT64 GetWorldPosBufferUavId() const { return m_worldPosUavId; }

};