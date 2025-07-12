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

#ifdef _DEBUG
    ComPtr<ID3D12Debug> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
    {
        debugController->EnableDebugLayer();
    }
#endif

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

    m_adapterNodeMask = adapterIndex;

    if (FAILED(D3D12CreateDevice(
        adapter,
        D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(m_pDevice.GetAddressOf()))))
    {
        return false;
    }

    // Create and init Descriptor heap manager
    m_descriptorHeapManager = std::unique_ptr<DescriptorHeapManager>(new DescriptorHeapManager(m_pDevice.Get(), m_adapterNodeMask));
    if (!m_descriptorHeapManager->Initialize())
    {
        return false;
    }

    // Query the descriptor sizes of current device.
    m_rtvDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    m_dsvDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    m_cbvSrvDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    m_samplerDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

    // Create direct command queue
    m_swapchainCommandQueue = std::unique_ptr<CommandQueue>(new CommandQueue(D3D12_COMMAND_QUEUE_FLAG_NONE, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_QUEUE_PRIORITY_NORMAL, adapterIndex));
    if (!m_swapchainCommandQueue->Initialize(m_pDevice.Get()))
    {
        return false;
    }

    m_swapchainCommandBuilder = std::unique_ptr<CommandBuilder>(new CommandBuilder(D3D12_COMMAND_LIST_TYPE_DIRECT));
    if (!m_swapchainCommandBuilder->Initialize(m_pDevice.Get()))
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
        m_swapchainCommandQueue->GetCommandQueue(),        // Swap chain needs the queue so that it can force a flush on it.
        hwnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        swapchain1.GetAddressOf())))
    {
        return false;
    }

    if (FAILED(swapchain1->QueryInterface(m_swapchain.GetAddressOf())))
    {
        return false;
    }

    for (UINT i = 0; i < m_numFrameBuffers; i++)
    {
        if (FAILED(m_swapchain->GetBuffer(i, IID_PPV_ARGS(m_swapchainBuffers[i].GetAddressOf()))))
        {
            return false;
        }

        uint64_t rtvId = m_descriptorHeapManager->CreateRenderTargetView(m_swapchainBuffers[i].Get(), nullptr);
        m_rtvIds[i] = rtvId;
    }

    RECT rect;
    GetWindowRect(hwnd, &rect);
    m_width = rect.right - rect.left;
    m_height = rect.bottom - rect.top;
    
    // Create the viewport.
    m_viewport.TopLeftX = 0.0f;
    m_viewport.TopLeftY = 0.0f;
    m_viewport.Width = static_cast<float>(m_width);
    m_viewport.Height = static_cast<float>(m_height);
    m_viewport.MinDepth = 0.0f;
    m_viewport.MaxDepth = 1.0f;

    // Create the scissor rectangles.
    m_scissorRect = { 0, 0, (long)m_width, (long)m_height };

    m_hwnd = hwnd;
    m_initialized = true;

    return true;
}

bool GraphicsContext::CopyToCurrentBackBuffer()
{
    m_swapchainCommandBuilder->Reset();

    ID3D12GraphicsCommandList* commandList = m_swapchainCommandBuilder->GetCommandList();

    D3D12_CPU_DESCRIPTOR_HANDLE rtv;
    m_descriptorHeapManager->GetRenderTargetViewCpuHandle(m_rtvIds[m_currentBackbuffer], rtv);
    float clearColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
    
    D3D12_RESOURCE_BARRIER transitionFromPresentToRenderTarget = CD3DX12_RESOURCE_BARRIER::Transition(
        m_swapchainBuffers[m_currentBackbuffer].Get(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET
    );

    commandList->ResourceBarrier(1, &transitionFromPresentToRenderTarget);
    commandList->OMSetRenderTargets(1, &rtv, false, nullptr);
    commandList->ClearRenderTargetView(rtv, clearColor, 0, NULL);

    D3D12_RESOURCE_BARRIER transitionFromRenderTargetToPresent = CD3DX12_RESOURCE_BARRIER::Transition(
        m_swapchainBuffers[m_currentBackbuffer].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT
    );

    commandList->ResourceBarrier(1, &transitionFromRenderTargetToPresent);

    m_swapchainCommandBuilder->Close();
    m_swapchainCommandQueue->DispatchCommands(m_swapchainCommandBuilder.get());
    m_swapchainCommandQueue->SignalAndWait();

    return true;
}

bool GraphicsContext::PresentCurrentBackBuffer()
{
    m_currentBackbuffer = (m_currentBackbuffer + 1) % m_numFrameBuffers;

    return SUCCEEDED(m_swapchain->Present(0, 0));

}

GraphicsContext::~GraphicsContext()
{
    if (m_idxgiFactory)
    {
        m_idxgiFactory.Reset();
    }

    if (m_pDevice)
    {
        m_pDevice.Reset();
    }

    if (m_swapchain)
    {
        m_swapchain.Reset();
    }
}