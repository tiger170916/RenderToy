#pragma once

#include "Includes.h"
#include "D3DResource.h"
#include "GraphicsContext.h"

template <typename T>
class ConstantBuffer
{
private:
	std::unique_ptr<D3DResource> m_resource = nullptr;

	std::vector<T> m_buffer;

	UINT m_numInstances = 1;

	UINT64 m_cbvId = UINT64_MAX;

	bool m_initialized = false;

	bool m_dirty = false;

public:
	ConstantBuffer(UINT numInstances = 1)
	{
		if (numInstances < 1)
		{
			numInstances = 1;
		}

		m_buffer = std::vector<T>(numInstances, T());

		m_numInstances = numInstances;
	}

	inline void MarkDirty()
	{
		m_dirty = true;
	}

	T& operator [](int idx) {
		m_dirty = true;
		return m_buffer[idx];
	}

	T operator [](int idx) const {
		return m_buffer[idx];
	}

	inline bool BindConstantBufferViewToPipeline(GraphicsContext* graphicsContext, D3D12_GPU_DESCRIPTOR_HANDLE& gpuDescriptorHandle)
	{
		if (!graphicsContext)
		{
			return false;
		}

		DescriptorHeapManager* descriptorHeapManager = graphicsContext->GetDescriptorHeapManager();

		return descriptorHeapManager->BindCbvSrvUavToPipeline(m_cbvId, gpuDescriptorHandle);
	}

	bool Initialize(GraphicsContext* graphicsContext)
	{
		if (m_initialized)
		{
			return true;
		}

		if (!graphicsContext)
		{
			return false;
		}

		ID3D12Device* pDevice = graphicsContext->GetDevice();
		DescriptorHeapManager* descriptorHeapManager = graphicsContext->GetDescriptorHeapManager();

		UINT bufferSize = (sizeof(T) * m_numInstances + 255) & ~255;
		m_resource = std::unique_ptr<D3DResource>(new D3DResource(false));

		auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
		if (!m_resource->Initialize(graphicsContext, &bufferDesc, m_buffer.data(), bufferSize, 0))
		{
			return false;
		}

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		// Use default buffer if the data is static.
		cbvDesc.BufferLocation = m_resource->GetUploadResource()->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = bufferSize;

		m_cbvId = descriptorHeapManager->CreateConstantBufferView(&cbvDesc);

		m_initialized = true;
		m_dirty = true;
		return true;
	}

	bool UpdateToGPU()
	{
		if (!m_initialized)
		{
			return false;
		}

		if (!m_dirty)
		{
			return true;
		}

		if (!m_resource)
		{
			return false;
		}

		return m_resource->UpdateUploadBuffer(m_buffer.data(), m_numInstances * sizeof(T));
	}

	inline const bool IsInitialized() const { return m_initialized; }

	~ConstantBuffer()
	{
		
	}
};