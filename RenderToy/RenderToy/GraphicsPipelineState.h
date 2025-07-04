#pragma once

#include "Includes.h"
#include "ShaderType.h"

class GraphicsPipelineState
{
private:
	ComPtr<ID3D12RootSignature> m_rootSignature = nullptr;

	ComPtr<ID3D12PipelineState> m_graphicsPipelineState = nullptr;

public:
	GraphicsPipelineState(
		ID3D12Device *pDevice, D3D12_INPUT_ELEMENT_DESC* inputLayout, UINT numInputs, ShaderType vs, ShaderType ps);
};