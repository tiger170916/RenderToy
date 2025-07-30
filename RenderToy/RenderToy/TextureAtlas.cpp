#include "TextureAtlas.h"

TextureAtlas::TextureAtlas() {}

bool TextureAtlas::Initialize(GraphicsContext* graphicsContext, UINT textureSize, const std::vector<UINT>& levels, D3D12_RESOURCE_DESC* textureDesc)
{
	if (m_initialized)
	{
		return false;
	}

	// Validate sizes
	if (levels.size() == 0 || textureSize == 0)
	{
		return false;
	}

	m_textureSize = textureSize;
	m_levelSizes.push_back(textureSize);
	for (auto& size : levels)
	{
		m_levelSizes.push_back(size);
	}

	for (int i = 0; i < m_levelSizes.size() - 1; i++)
	{
		if (m_levelSizes[i] % m_levelSizes[i + 1] > 0)
		{
			return false;
		}
	}

	// Valid ... 
	PushNodes(m_nodes, 0, 0, 0);

	ID3D12Device* pDevice = graphicsContext->GetDevice();
	// Initialize texture
	D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	if (FAILED(pDevice->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
		textureDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(m_pResource.GetAddressOf()))))
	{
		return false;
	}

	m_textureFormat = textureDesc->Format;

	m_currentState = D3D12_RESOURCE_STATE_COMMON;

	m_initialized = true;
	return true;
}

bool TextureAtlas::RequestNode(UINT level, void* extension, Node& outNode)
{
	if (level >= m_levelSizes.size() || level == 0)
	{
		return false;
	}

	NodeInternal* result = RequestNodeInternal(m_nodes, 1, level);
	if (!result)
	{
		return false;
	}

	result->ExtensionInfo = extension;
	result->IsEmpty = false;

	outNode.ExtensionInfo = extension;
	outNode.OffsetX = result->OffsetX;
	outNode.OffsetY = result->OffsetY;
	outNode.Size = result->Size;

	return true;
}

TextureAtlas::NodeInternal* TextureAtlas::RequestNodeInternal(std::vector<std::vector<std::unique_ptr<NodeInternal>>>& nodes, UINT currentLevel, UINT targetLevel)
{
	for (size_t i = 0; i < nodes.size(); i++)
	{
		for (size_t j = 0; j < nodes.size(); j++)
		{
			if (targetLevel == currentLevel)
			{
				if (nodes[i][j]->IsEmpty)
				{
					return nodes[i][j].get();
				}
			}
			
			NodeInternal* found = RequestNodeInternal(nodes[i][j]->SubLayers, currentLevel + 1, targetLevel);
			if (found)
			{
				return found;
			}
		}
	}

	return nullptr;
}

void TextureAtlas::PushNodes(std::vector<std::vector<std::unique_ptr<NodeInternal>>>& nodes, UINT currentLevelIdx, UINT currentNodeOffsetX, UINT currentNodeOffsetY)
{
	if (currentLevelIdx == m_levelSizes.size() - 1)
	{
		return;
	}

	UINT currentLevelSize = m_levelSizes[currentLevelIdx];
	UINT nextLevelSize = m_levelSizes[currentLevelIdx + 1];
	UINT nextLevelCount = currentLevelSize / nextLevelSize;

	for (UINT i = 0; i < nextLevelCount; i++)
	{
		nodes.push_back(std::vector<std::unique_ptr<NodeInternal>>());
		for (UINT j = 0; j < nextLevelCount; j++)
		{
			NodeInternal* newNode = new NodeInternal();
			newNode->IsEmpty = true;
			newNode->ExtensionInfo = nullptr;
			newNode->Size = nextLevelSize;
			newNode->OffsetX = currentNodeOffsetX + nextLevelSize * i;
			newNode->OffsetY = currentNodeOffsetY + nextLevelSize * j;

			nodes[i].push_back(std::unique_ptr<NodeInternal>(newNode));
			PushNodes(newNode->SubLayers, currentLevelIdx + 1, newNode->OffsetX, newNode->OffsetY);
		}
	}
}

void TextureAtlas::ClearNodes()
{
	ClearNodesInternal(m_nodes);
}

void TextureAtlas::ClearNodesInternal(std::vector<std::vector<std::unique_ptr<NodeInternal>>>& nodes)
{
	for (size_t i = 0; i < nodes.size(); i++)
	{
		for (size_t j = 0; j < nodes.size(); j++)
		{
			nodes[i][j]->ExtensionInfo = nullptr;
			nodes[i][j]->IsEmpty = true;

			ClearNodesInternal(nodes[i][j]->SubLayers);
		}
	}
}

TextureAtlas::~TextureAtlas() {}