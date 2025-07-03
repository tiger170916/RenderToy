#include "GraphicsContext.h"

#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3d12.lib")

GraphicsContext::GraphicsContext()
{

}

bool GraphicsContext::Initialize(HWND hwnd)
{
	if (m_initialized)
	{
		return true;
	}

    if (!IsWindow(hwnd))
    {
        return false;
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

    // Create direct command queue
    m_directCommandQueue = std::unique_ptr<CommandQueue>(new CommandQueue(D3D12_COMMAND_QUEUE_FLAG_NONE, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_QUEUE_PRIORITY_NORMAL, adapterIndex));
    if (!m_directCommandQueue->Initialize(m_pDevice.Get()))
    {
        return false;
    }

    // Create swapchain.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = m_numFrameBuffers;
    swapChainDesc.Width = 0;
    swapChainDesc.Height = 0;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> swapchain1 = nullptr;
    if (FAILED(m_idxgiFactory->CreateSwapChainForHwnd(
        m_directCommandQueue->GetCommandQueue(),        // Swap chain needs the queue so that it can force a flush on it.
        hwnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        swapchain1.GetAddressOf())))
    {
        return false;
    }

    m_hwnd = hwnd;
    m_initialized = true;

    return true;
}


GraphicsContext::~GraphicsContext()
{
    if (m_idxgiFactory)
    {
        m_idxgiFactory->Release();
        m_idxgiFactory = nullptr;
    }

    if (m_pDevice)
    {
        m_pDevice->Release();
        m_pDevice = nullptr;
    }
}