#pragma once

#include "Includes.h"
#include "ShaderType.h"

class GraphicsPipelineState
{
private:
	ComPtr<ID3D12RootSignature> m_rootSignature = nullptr;

	ComPtr<ID3D12PipelineState> m_graphicsPipelineState = nullptr;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc {};

public:
	GraphicsPipelineState();

	~GraphicsPipelineState();

	bool Initialize(ID3D12Device* pDevice);

	inline D3D12_GRAPHICS_PIPELINE_STATE_DESC& GraphicsPipelineStateDesc() { return desc; }
};