#pragma once

#include "Includes.h"
#include "D3DResource.h"
#include "DescriptorHeapManager.h"

template <typename T>
class ConstantBuffer
{
private:
	std::unique_ptr<D3DResource> m_resource = nullptr;

	std::vector<T> m_buffer;

	UINT m_numInstances = 1;

	UINT64 m_cbvId = UINT64_MAX;

	bool m_initialized = false;

public:
	ConstantBuffer(UINT numInstances = 1)
	{
		if (numInstances < 1)
		{
			numInstances = 1;
		}

		m_buffer = std::vector<T>(100, T());

		m_numInstances = numInstances;
	}



	T& operator [](int idx) {
		return m_buffer[idx];
	}

	T operator [](int idx) const {
		return m_buffer[idx];
	}

	inline bool BindConstantBufferViewToPipeline(DescriptorHeapManager* descriptorHeapManager, D3D12_GPU_DESCRIPTOR_HANDLE& gpuDescriptorHandle)
	{
		return descriptorHeapManager->BindCbvSrvUavToPipeline(m_cbvId, gpuDescriptorHandle);
	}

	bool Initialize(ID3D12Device* pDevice, DescriptorHeapManager* descriptorHeapManager)
	{
		if (m_initialized)
		{
			return true;
		}

		if (!pDevice || !descriptorHeapManager)
		{
			return false;
		}

		UINT bufferSize = (sizeof(T) * m_numInstances + 255) & ~255;
		m_resource = std::unique_ptr<D3DResource>(new D3DResource(false));

		auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
		if (!m_resource->Initialize(pDevice, &bufferDesc, m_buffer.data(), bufferSize))
		{
			return false;
		}

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		// Use default buffer if the data is static.
		cbvDesc.BufferLocation = m_resource->GetUploadResource()->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = bufferSize;

		m_cbvId = descriptorHeapManager->CreateConstantBufferView(&cbvDesc);

		m_initialized = true;
		return true;
	}

	bool UpdateToGPU()
	{
		if (!m_initialized)
		{
			return false;
		}

		if (!m_resource)
		{
			return false;
		}

		return m_resource->UpdateUploadBuffer(m_buffer.data(), m_numInstances * sizeof(T));
	}

	~ConstantBuffer()
	{
		
	}
};