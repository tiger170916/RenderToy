#include "D3DResource.h"

D3DResource::D3DResource(bool needCopyToDefaultHeap)
	: m_needCopyToDefaultHeap(needCopyToDefaultHeap)
{
}

bool D3DResource::Initialize(ID3D12Device* pDevice, const D3D12_RESOURCE_DESC* pResourceDesc, void* data, UINT dataSize)
{
	if (m_initialized)
	{
		return true;
	}

	if (pDevice == nullptr || pResourceDesc == nullptr || data == nullptr || dataSize <= 0)
	{
		return false;
	}

	auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

	// create upload heap
		// upload heaps are used to upload data to the GPU. CPU can write to it, GPU can read from it
		// We will upload the vertex buffer using this heap to the default heap
	if (FAILED(pDevice->CreateCommittedResource(
		&uploadHeapProperties, // upload heap
		D3D12_HEAP_FLAG_NONE, // no flags
		pResourceDesc, // resource description for a buffer
		D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
		nullptr,
		IID_PPV_ARGS(m_uploadeHeapResource.GetAddressOf()))))
	{
		return false;
	}

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
			D3D12_RESOURCE_STATE_COPY_DEST, // we will start this heap in the copy destination state since we will copy data
			// from the upload heap to this heap
			nullptr, // optimized clear value must be null for this type of resource. used for render targets and depth/stencil buffers
			IID_PPV_ARGS(m_defaultHeapResource.GetAddressOf()))))
		{
			return false;
		}
	}

	m_initialized = true;

	return true;
}

bool D3DResource::UpdateUploadBuffer(void* data, UINT size)
{
	if (m_uploadeHeapResource == nullptr)
	{
		return false;
	}

	void* pMappedData;
	D3D12_RANGE readRange = { 0, 0 };
	if (FAILED(m_uploadeHeapResource->Map(0, &readRange, &pMappedData)))
	{
		return false;
	}

	memcpy(pMappedData, data, size);

	D3D12_RANGE writeRange = { 0, size }; // The range that was written to
	m_uploadeHeapResource->Unmap(0, &writeRange);

	return true;
}

D3DResource::~D3DResource()
{
	if (m_uploadeHeapResource)
	{
		m_uploadeHeapResource->Release();
		m_uploadeHeapResource = nullptr;
	}

	if (m_defaultHeapResource)
	{
		m_defaultHeapResource->Release();
		m_defaultHeapResource = nullptr;
	}
}