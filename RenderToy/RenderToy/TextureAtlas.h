#pragma once

#include "Includes.h"
#include "GraphicsContext.h"

class TextureAtlas
{
private:
	struct NodeInternal
	{
		UINT Size;
		UINT OffsetX;
		UINT OffsetY;
		void* ExtensionInfo;

		bool IsEmpty;

		std::vector<std::vector<std::unique_ptr<NodeInternal>>> SubLayers;
	};

public:
	struct Node
	{
		UINT Size;
		UINT OffsetX;
		UINT OffsetY;

		void* ExtensionInfo;
	};

private:
	ComPtr<ID3D12Resource> m_pResource = nullptr;

	DXGI_FORMAT m_textureFormat = DXGI_FORMAT_UNKNOWN;

	D3D12_RESOURCE_STATES m_currentState = D3D12_RESOURCE_STATE_COMMON;

	UINT m_textureSize = 0;

	std::vector<UINT> m_levelSizes;

	bool m_initialized = false;

	std::vector<std::vector<std::unique_ptr<NodeInternal>>> m_nodes;

public:
	TextureAtlas();

	bool Initialize(GraphicsContext* graphicsContext, UINT textureSize, const std::vector<UINT>& levels, D3D12_RESOURCE_DESC* textureDesc);

	void ClearNodes();

	bool RequestNode(UINT level, void* extension, Node& outNode);

	inline const bool IsInitialize() const { return m_initialized; }

	inline ID3D12Resource* GetResource() { return m_pResource.Get(); }

	~TextureAtlas();

private:
	void PushNodes(std::vector<std::vector<std::unique_ptr<NodeInternal>>>& nodes, UINT currentLevelIdx, UINT currentNodeOffsetX, UINT currentNodeOffsetY);

	void ClearNodesInternal(std::vector<std::vector<std::unique_ptr<NodeInternal>>>& nodes);

	NodeInternal* RequestNodeInternal(std::vector<std::vector<std::unique_ptr<NodeInternal>>>& nodes, UINT currentLevel, UINT targetLevel);
};