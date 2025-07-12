#include "GraphicsPipelineState.h"

GraphicsPipelineState::GraphicsPipelineState() {}

GraphicsPipelineState::~GraphicsPipelineState() {}

bool GraphicsPipelineState::Initialize(
	GraphicsContext* graphicsContext,
	ShaderManager* shaderManager,
	ShaderType rootSignature,
	ShaderType vertexShader,
	ShaderType pixelShader)
{
	if (!graphicsContext || !shaderManager)
	{
		return false;
	}

	ID3D12Device* pDevice = graphicsContext->GetDevice();
	UINT adapterNodeMask = graphicsContext->GetAdapterNodeMask();

	char* rootSignatureData = nullptr;
	UINT rootSignatureSize = 0;
	if (!shaderManager->GetShader(rootSignature, &rootSignatureData, rootSignatureSize))
	{
		return false;
	}

	if (FAILED(pDevice->CreateRootSignature(adapterNodeMask, rootSignatureData, rootSignatureSize, IID_PPV_ARGS(m_rootSignature.GetAddressOf()))))
	{
		return false;
	}

	m_desc.pRootSignature = m_rootSignature.Get();

	char* shaderData = nullptr;
	UINT shaderSize = 0;
	if (vertexShader != ShaderType::SHADER_TYPE_NONE)
	{
		if (!shaderManager->GetShader(vertexShader, &shaderData, shaderSize))
		{
			return false;
		}

		m_desc.VS.BytecodeLength = shaderSize;
		m_desc.VS.pShaderBytecode = shaderData;
	}

	if (pixelShader != ShaderType::SHADER_TYPE_NONE)
	{
		if (!shaderManager->GetShader(pixelShader, &shaderData, shaderSize))
		{
			return false;
		}

		m_desc.PS.BytecodeLength = shaderSize;
		m_desc.PS.pShaderBytecode = shaderData;
	}

	if (FAILED(pDevice->CreateGraphicsPipelineState(&m_desc, IID_PPV_ARGS(m_graphicsPipelineState.GetAddressOf()))))
	{
		return false;
	}

	return true;
}