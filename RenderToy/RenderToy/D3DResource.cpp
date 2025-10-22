#include "D3DResource.h"
#include "GraphicsUtils.h"
#include "LightStructs.h"

D3DResource::D3DResource(bool needCopyToDefaultHeap)
	: m_needCopyToDefaultHeap(needCopyToDefaultHeap)
{
}

bool D3DResource::Initialize(GraphicsContext* graphicsContext, D3D12_RESOURCE_DESC* pResourceDesc, void* data, UINT dataSize, UINT stride)
{
	if (m_initialized)
	{
		return true;
	}

	if (graphicsContext == nullptr || pResourceDesc == nullptr || data == nullptr || dataSize <= 0)
	{
		return false;
	}

	ID3D12Device* pDevice = graphicsContext->GetDevice();

	auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

	bool isTexture = pResourceDesc->Dimension != D3D12_RESOURCE_DIMENSION_BUFFER;

	m_stride = stride;

	UINT64 width = pResourceDesc->Width;
	UINT height = pResourceDesc->Height;
	UINT16 depth = pResourceDesc->DepthOrArraySize;
	DXGI_FORMAT format = pResourceDesc->Format;
	D3D12_RESOURCE_DIMENSION dimension = pResourceDesc->Dimension;
	D3D12_TEXTURE_LAYOUT layout = pResourceDesc->Layout;
	if (isTexture)
	{
		// For texture resource, use buffer for upload heap
		pResourceDesc->Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		pResourceDesc->Width = dataSize;
		pResourceDesc->Height = 1;
		pResourceDesc->DepthOrArraySize = 1;
		pResourceDesc->Format = DXGI_FORMAT_UNKNOWN;
		pResourceDesc->Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		m_isTexture = true;
		m_width = (UINT)width;
		m_height = height;
		m_depth = (UINT)depth;
		m_dxgiFormat = format;
	}

	// create upload heap
		// upload heaps are used to upload data to the GPU. CPU can write to it, GPU can read from it
		// We will upload the vertex buffer using this heap to the default heap
	if (FAILED(pDevice->CreateCommittedResource(
		&uploadHeapProperties, // upload heap
		D3D12_HEAP_FLAG_NONE, // no flags
		pResourceDesc, // resource description for a buffer
		D3D12_RESOURCE_STATE_COMMON, // GPU will read from this buffer and copy its contents to the default heap
		nullptr,
		IID_PPV_ARGS(m_uploadHeapResource.GetAddressOf()))))
	{
		return false;
	}

	if (isTexture)
	{
		// Change the desc back
		pResourceDesc->Dimension = dimension;
		pResourceDesc->Width = width;
		pResourceDesc->Height = height;
		pResourceDesc->DepthOrArraySize = depth;
		pResourceDesc->Format = format;
		pResourceDesc->Layout = layout;
	}

	m_uploadBufferCurrentState = D3D12_RESOURCE_STATE_COMMON;

	if (!UpdateUploadBuffer(data, dataSize))
	{
		return false;
	}

	if (m_needCopyToDefaultHeap)
	{
		auto defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		ComPtr<ID3D12Resource> vbDefaultResource = nullptr;
		if (FAILED(pDevice->CreateCommittedResource(
			&defaultHeapProperties, // a default heap
			D3D12_HEAP_FLAG_NONE, // no flags
			pResourceDesc, // resource description for a buffer
			D3D12_RESOURCE_STATE_COMMON, // we will start this heap in the copy destination state since we will copy data
			// from the upload heap to this heap
			nullptr, // optimized clear value must be null for this type of resource. used for render targets and depth/stencil buffers
			IID_PPV_ARGS(m_defaultHeapResource.GetAddressOf()))))
		{
			return false;
		}

		m_defaultBufferCurrentState = D3D12_RESOURCE_STATE_COMMON;
	}

	m_initialized = true;

	return true;
}

bool D3DResource::UpdateUploadBuffer(void* data, UINT size)
{
	if (m_uploadHeapResource == nullptr)
	{
		return false;
	}

	void* pMappedData;
	D3D12_RANGE readRange = { 0, size };
	if (FAILED(m_uploadHeapResource->Map(0, &readRange, &pMappedData)))
	{
		return false;
	}

	memcpy(pMappedData, data, size);

	D3D12_RANGE writeRange = { 0, size }; // The range that was written to
	m_uploadHeapResource->Unmap(0, &writeRange);

	m_copiedToDefaultHeap = false;

	return true;
}

bool D3DResource::CopyToDefaultHeap(ID3D12GraphicsCommandList* commandList)
{
	if (m_copiedToDefaultHeap)
	{
		return true;
	}

	if (!commandList)
	{
		return false;
	}

	if (!m_needCopyToDefaultHeap || !m_defaultHeapResource)
	{
		return false;
	}

	D3D12_BOX textureBox
	{
		.left = 0,
		.top = 0,
		.front = 0,
		.right = static_cast<UINT>(m_width),
		.bottom = static_cast<UINT>(m_height),
		.back = 1
	};

	if (m_isTexture)
	{
		D3D12_TEXTURE_COPY_LOCATION srcLoc{}, destLoc{};
		srcLoc.pResource = m_uploadHeapResource.Get();
		srcLoc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		srcLoc.PlacedFootprint.Offset = 0;
		srcLoc.PlacedFootprint.Footprint.Width = m_width;
		srcLoc.PlacedFootprint.Footprint.Height = m_height;
		srcLoc.PlacedFootprint.Footprint.Depth = 1;
		srcLoc.PlacedFootprint.Footprint.RowPitch = m_stride;
		srcLoc.PlacedFootprint.Footprint.Format = m_dxgiFormat;

		destLoc.pResource = m_defaultHeapResource.Get();
		destLoc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		destLoc.SubresourceIndex = 0;

		commandList->CopyTextureRegion(&destLoc, 0, 0, 0, &srcLoc, &textureBox);
	}
	else
	{
		commandList->CopyResource(m_defaultHeapResource.Get(), m_uploadHeapResource.Get());
	}

	m_copiedToDefaultHeap = true;

	return true;
}

bool D3DResource::CleanUploadResource()
{
	// Do not clean the upload heap if there is not a default heap
	if (!m_needCopyToDefaultHeap)
	{
		return false;
	}

	m_uploadHeapResource.Reset();

	return true;
}

D3DResource::~D3DResource()
{
	if (m_uploadHeapResource)
	{
		m_uploadHeapResource.Reset();
	}

	if (m_defaultHeapResource)
	{
		m_defaultHeapResource.Reset();
	}
}