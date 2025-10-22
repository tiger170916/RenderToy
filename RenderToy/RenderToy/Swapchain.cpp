#include "Swapchain.h"
#include "DescriptorHeapManager.h"
#include "GraphicsUtils.h"

Swapchain::Swapchain() {}

Swapchain::~Swapchain() {}

bool Swapchain::Initialize(GraphicsContext* graphicsContext)
{
    if (!graphicsContext)
    {
        return false;
    }

    ID3D12Device* pDevice = graphicsContext->GetDevice();
    UINT adapterIndex = graphicsContext->GetAdapterNodeMask();
    IDXGIFactory2* idxgiFactory = graphicsContext->GetDxgiFactory();
    HWND hwnd = graphicsContext->GetHwnd();
    const UINT width = graphicsContext->GetHwndWidth();
    const UINT height = graphicsContext->GetHwndHeight();

    DescriptorHeapManager* descriptorHeapManager = graphicsContext->GetDescriptorHeapManager();

    // Create direct command queue
    m_swapchainCommandQueue = std::unique_ptr<CommandQueue>(new CommandQueue(D3D12_COMMAND_QUEUE_FLAG_NONE, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_QUEUE_PRIORITY_NORMAL, adapterIndex));
    if (!m_swapchainCommandQueue->Initialize(pDevice))
    {
        return false;
    }

    m_swapchainCommandBuilder = std::unique_ptr<CommandBuilder>(new CommandBuilder(D3D12_COMMAND_LIST_TYPE_DIRECT));
    if (!m_swapchainCommandBuilder->Initialize(pDevice))
    {
        return false;
    }

    // Create swapchain.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = m_numFrameBuffers;
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> swapchain1 = nullptr;
    if (FAILED(idxgiFactory->CreateSwapChainForHwnd(
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

        uint64_t rtvId = descriptorHeapManager->CreateRenderTargetView(m_swapchainBuffers[i].Get(), nullptr);
        m_rtvIds[i] = rtvId;
    }

    m_initialized = true;
    return true;
}

bool Swapchain::CopyToBackbuffer(GraphicsContext* graphicsContext, ID3D12Resource* resource)
{
    if (!graphicsContext || !resource)
    {
        return false;
    }

    DescriptorHeapManager* descHeapMgr = graphicsContext->GetDescriptorHeapManager();
    D3D12_CPU_DESCRIPTOR_HANDLE rtv;
    descHeapMgr->GetRenderTargetViewCpuHandle(m_rtvIds[m_currentBackbuffer], rtv);

    m_swapchainCommandBuilder->TryReset();

    ID3D12GraphicsCommandList* commandList = m_swapchainCommandBuilder->GetCommandList();

    GraphicsUtils::ResourceBarrierTransition(resource, m_swapchainCommandBuilder->GetCommandList(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_SOURCE);
    GraphicsUtils::ResourceBarrierTransition(m_swapchainBuffers[m_currentBackbuffer].Get(), commandList, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList->ClearRenderTargetView(rtv, m_rtClearColor, 0, NULL);


    GraphicsUtils::ResourceBarrierTransition(m_swapchainBuffers[m_currentBackbuffer].Get(), commandList, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST);
    commandList->CopyResource(m_swapchainBuffers[m_currentBackbuffer].Get(), resource);
    GraphicsUtils::ResourceBarrierTransition(m_swapchainBuffers[m_currentBackbuffer].Get(), commandList, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT);
    GraphicsUtils::ResourceBarrierTransition(resource, m_swapchainCommandBuilder->GetCommandList(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON);

    m_swapchainCommandBuilder->Close();
    m_swapchainCommandQueue->DispatchCommands(m_swapchainCommandBuilder.get());
    m_swapchainCommandQueue->SignalAndWait();

    return true;
}

bool Swapchain::Present()
{
    m_currentBackbuffer = (m_currentBackbuffer + 1) % m_numFrameBuffers;

    return SUCCEEDED(m_swapchain->Present(0, 0));
}