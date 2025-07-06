#pragma once

#include "Includes.h"
#include "D3DResource.h"

template <typename T>
class ConstantBuffer
{
private:
	std::unique_ptr<D3DResource> m_resource = nullptr;

	std::vector<T> m_buffer;

	UINT m_numInstances = 1;

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

	bool Initialize(ID3D12Device* pDevice)
	{
		UINT bufferSize = (sizeof(T) + 255) & ~255;
		m_resource = std::unique_ptr<D3DResource>(new D3DResource(false));

		auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
		if (!m_resource->Initialize(pDevice, &bufferDesc, m_buffer.data(), (UINT)m_buffer.size() * sizeof(T)))
		{
			return false;
		}

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

		m_resource->UpdateUploadBuffer(m_buffer.data(), m_buffer.size() * sizeof(T));
	}

	~ConstantBuffer()
	{
		
	}
};