#pragma once

#include "Includes.h"
#include "ShaderType.h"
#include "ShaderManager.h"

class GraphicsPipelineState
{
private:
	ComPtr<ID3D12RootSignature> m_rootSignature = nullptr;

	ComPtr<ID3D12PipelineState> m_graphicsPipelineState = nullptr;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC m_desc {};

public:
	GraphicsPipelineState();

	~GraphicsPipelineState();

	bool Initialize(
		ID3D12Device* pDevice,
		UINT adapterNodeMask,
		ShaderManager* shaderManager,
		ShaderType rootSignature,
		ShaderType vertexShader,
		ShaderType pixelShader);

	inline D3D12_GRAPHICS_PIPELINE_STATE_DESC& GraphicsPipelineStateDesc() { return m_desc; }

	inline ID3D12PipelineState* GetPipelineState() const { return m_graphicsPipelineState.Get(); }

	inline ID3D12RootSignature* GetRootSignature() const { return m_rootSignature.Get(); }
};