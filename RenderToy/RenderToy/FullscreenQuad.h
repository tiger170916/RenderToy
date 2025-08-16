#pragma once

#include "Includes.h"
#include "D3DResource.h"
#include "GraphicsContext.h"

class FullScreenQuad
{
private:
	static std::unique_ptr<FullScreenQuad> _singleton;

	std::unique_ptr<D3DResource>  m_vbResource = nullptr;

	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView = {};

	const UINT m_vertexCount = 6;

	bool m_initialized = false;

private:
	FullScreenQuad() = default;

public:
	~FullScreenQuad();

	static FullScreenQuad* Get();

	bool Initialize(GraphicsContext* graphicsContext);

	bool Draw(ID3D12GraphicsCommandList* commandList);

};
