#pragma once

#include "Includes.h"
#include "ShaderType.h"
#include "ShaderManager.h"
#include "GraphicsContext.h"

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
		GraphicsContext* graphicsContext,
		ShaderManager* shaderManager,
		ShaderType rootSignature,
		ShaderType vertexShader,
		ShaderType geometryShader,
		ShaderType pixelShader);

	inline D3D12_GRAPHICS_PIPELINE_STATE_DESC& GraphicsPipelineStateDesc() { return m_desc; }

	inline ID3D12PipelineState* GetPipelineState() const { return m_graphicsPipelineState.Get(); }

	inline ID3D12RootSignature* GetRootSignature() const { return m_rootSignature.Get(); }
};