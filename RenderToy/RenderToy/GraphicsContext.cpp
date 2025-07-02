#include "GraphicsContext.h"

#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3d12.lib")

GraphicsContext::GraphicsContext()
{

}

bool GraphicsContext::Initialize()
{
	if (m_initialized)
	{
		return true;
	}

    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(m_idxgiFactory.GetAddressOf()))))
    {
        return false;
    }

    int adapterIndex = 0; // we'll start looking for directx 12  compatible graphics devices starting at index 0

    bool adapterFound = false; // set this to true when a good one was found

    // find first hardware gpu that supports d3d 12
    IDXGIAdapter1* adapter = nullptr;
    while (m_idxgiFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            // we dont want a software device
            adapterIndex++; // add this line here. Its not currently in the downloadable project
            continue;
        }

        // we want a device that is compatible with direct3d 12 (feature level 11 or higher)
        if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
        {
            adapterFound = true;
            break;
        }

        adapterIndex++;
    }

    if (!adapterFound)
    {
        return false;
    }

    if (FAILED(D3D12CreateDevice(
        adapter,
        D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(m_pDevice.GetAddressOf()))))
    {
        return false;
    }

    m_initialized = true;

    return true;
}


GraphicsContext::~GraphicsContext()
{

}