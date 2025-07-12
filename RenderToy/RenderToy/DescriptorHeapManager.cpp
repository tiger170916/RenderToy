#include "DescriptorHeapManager.h"

DescriptorHeapManager::DescriptorHeapManager(ID3D12Device* pDevice, UINT adapterNodeMask)
    : m_pDevice(pDevice), m_adapterNodeMask(adapterNodeMask)
{
}

bool DescriptorHeapManager::Initialize()
{
    if (m_initialized)
    {
        return true;
    }

    if (!m_pDevice)
    {
        return false;
    }

    // Get desc sizes of all types.
    m_rtvDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    m_dsvDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    m_cbvSrvUavDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    m_samplerDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

    // Create Rtv heap, this heap is used for all render targets, and swapchain buffers
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
    heapDesc.NumDescriptors = m_defaultRtvDescriptorHeapSize;
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    heapDesc.NodeMask = m_adapterNodeMask;
    if (FAILED(m_pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_rtvDescriptorHeap.GetAddressOf()))))
    {
        return false;
    }

    // Create Dsv heap, this heap is used for all render depth buffer
    heapDesc.NumDescriptors = m_defaultDsvDescriptorHeapSize;
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    if (FAILED(m_pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_dsvDescriptorHeap.GetAddressOf()))))
    {
        return false;
    }

    // Create sampler state descriptor heap, this heap is directly for shader usage
    heapDesc.NumDescriptors = m_samplerDescriptorSize;
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    if (FAILED(m_pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_samplerDescriptorHeap.GetAddressOf()))))
    {
        return false;
    }

    // Create cbv/srv/uav descriptor heap (ring buffer), this is directly used by shaders.
    heapDesc.NumDescriptors = m_defaultRingBufferSize;
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    if (FAILED(m_pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_cbvSrvUavRingBuffer.GetAddressOf()))))
    {
        return false;
    }

    // Initialize cbv/srv/uav id list, which indicates the position of gpu non-visible of the corresponding idx. 
    // (lower 32bit represents the index of vector, higher 32 bit represents the index of element within that vector, 0xffffffffffffffff means invalid)
    m_cbvSrvUavRingBufferIds = std::vector<uint64_t>(m_defaultRingBufferSize, UINT64_MAX);

    m_initialized = true;
    return true;
}

uint64_t DescriptorHeapManager::CreateRenderTargetView(ID3D12Resource* pResource, const D3D12_RENDER_TARGET_VIEW_DESC* pDesc)
{
    if (m_currentRtvCount >= m_defaultRtvDescriptorHeapSize)
    {
        return UINT64_MAX;
    }

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), m_currentRtvCount, m_rtvDescriptorSize);
    m_pDevice->CreateRenderTargetView(pResource, pDesc, rtvHeapHandle);

    return m_currentRtvCount++;
}

uint64_t DescriptorHeapManager::CreateDepthStencilView(ID3D12Resource* pResource, const D3D12_DEPTH_STENCIL_VIEW_DESC* pDesc)
{
    if (m_currentDsvCount >= m_defaultDsvDescriptorHeapSize)
    {
        return UINT64_MAX;
    }

    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHeapHandle(m_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), m_currentDsvCount, m_dsvDescriptorSize);
    m_pDevice->CreateDepthStencilView(pResource, pDesc, dsvHeapHandle);

    return m_currentDsvCount++;
}

uint64_t DescriptorHeapManager::CreateSampler(const D3D12_SAMPLER_DESC* pDesc)
{
    if (m_currentSamplerCount >= m_defaultSamplerDescriptorHeapSize)
    {
        return UINT64_MAX;
    }

    CD3DX12_CPU_DESCRIPTOR_HANDLE samplerHeapHandle(m_samplerDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), m_currentSamplerCount, m_samplerDescriptorSize);
    m_pDevice->CreateSampler(pDesc, samplerHeapHandle);

    return m_currentSamplerCount++;
}

bool DescriptorHeapManager::AddNewCbvSrvUavDescriptorHeap()
{
    if (m_cbvSrvUavDescriptorPositions.empty() || m_cbvSrvUavDescriptorPositions.back().size() >= m_defaultCbvSrvUavDescriptorHeapSize)
    {
        // Need to create a new shader non visible descriptor heap
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
        heapDesc.NumDescriptors = m_defaultCbvSrvUavDescriptorHeapSize;
        heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        heapDesc.NodeMask = m_adapterNodeMask;
        ComPtr<ID3D12DescriptorHeap> newDescriptorHeap = nullptr;
        if (FAILED(m_pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(newDescriptorHeap.GetAddressOf()))))
        {
            return false;
        }

        m_cbvSrvUavDescriptorHeaps.push_back(newDescriptorHeap);
        m_cbvSrvUavDescriptorPositions.push_back(std::vector<int>());
    }

    return true;
}

uint64_t DescriptorHeapManager::GetIdOfCbvSrvUav(uint64_t arrayIdx, uint64_t elementIdx)
{
    uint64_t id = 0;

    id = id | arrayIdx << 32;
    id = id | elementIdx;

    return id;
}

void DescriptorHeapManager::GetArrayIdxAndElementIdxByViewId(uint64_t viewId, uint64_t& arrayIdx, uint64_t& elementIdx)
{
    arrayIdx = viewId >> 32;
    elementIdx = viewId << 32;
    elementIdx = elementIdx >> 32;
}

uint64_t DescriptorHeapManager::CreateConstantBufferView(const D3D12_CONSTANT_BUFFER_VIEW_DESC* pDesc)
{
    if (!AddNewCbvSrvUavDescriptorHeap())
    {
        return UINT64_MAX;
    }

    CD3DX12_CPU_DESCRIPTOR_HANDLE heapHandle(m_cbvSrvUavDescriptorHeaps.back()->GetCPUDescriptorHandleForHeapStart(), (UINT)m_cbvSrvUavDescriptorPositions.back().size(), m_cbvSrvUavDescriptorSize);
    m_cbvSrvUavDescriptorPositions.back().push_back(INT32_MAX);
    m_pDevice->CreateConstantBufferView(pDesc, heapHandle);

    // Figure id of the cbv/srv/uav
    return GetIdOfCbvSrvUav(m_cbvSrvUavDescriptorHeaps.size() - 1, m_cbvSrvUavDescriptorPositions.back().size() - 1);
}

uint64_t DescriptorHeapManager::CreateShaderResourceView(ID3D12Resource* pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc)
{
    if (!AddNewCbvSrvUavDescriptorHeap())
    {
        return UINT64_MAX;
    }

    CD3DX12_CPU_DESCRIPTOR_HANDLE heapHandle(m_cbvSrvUavDescriptorHeaps.back()->GetCPUDescriptorHandleForHeapStart(), (UINT)m_cbvSrvUavDescriptorPositions.back().size(), m_cbvSrvUavDescriptorSize);
    m_cbvSrvUavDescriptorPositions.back().push_back(INT32_MAX);
    m_pDevice->CreateShaderResourceView(pResource, pDesc, heapHandle);

    // Figure id of the cbv/srv/uav
    return GetIdOfCbvSrvUav(m_cbvSrvUavDescriptorHeaps.size() - 1, m_cbvSrvUavDescriptorPositions.back().size() - 1);
}

uint64_t DescriptorHeapManager::CreateUnorderedAccessView(ID3D12Resource* pResource, ID3D12Resource* pCounterResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* pDesc)
{
    if (!AddNewCbvSrvUavDescriptorHeap())
    {
        return UINT64_MAX;
    }

    CD3DX12_CPU_DESCRIPTOR_HANDLE heapHandle(m_cbvSrvUavDescriptorHeaps.back()->GetCPUDescriptorHandleForHeapStart(), (UINT)m_cbvSrvUavDescriptorPositions.back().size(), m_cbvSrvUavDescriptorSize);
    m_cbvSrvUavDescriptorPositions.back().push_back(INT32_MAX);
    m_pDevice->CreateUnorderedAccessView(pResource, pCounterResource, pDesc, heapHandle);

    // Figure id of the cbv/srv/uav
    return GetIdOfCbvSrvUav(m_cbvSrvUavDescriptorHeaps.size() - 1, m_cbvSrvUavDescriptorPositions.back().size() - 1);
}

bool DescriptorHeapManager::BindCbvSrvUavToPipeline(uint64_t viewId, D3D12_GPU_DESCRIPTOR_HANDLE& outGpuDescriptorHandle)
{
    // Figure of arrayIdx and elementIdx from id
    uint64_t arrayIdx = 0;
    uint64_t elementIdx = 0;
    GetArrayIdxAndElementIdxByViewId(viewId, arrayIdx, elementIdx);

    if (m_cbvSrvUavDescriptorPositions.size() <= arrayIdx || m_cbvSrvUavDescriptorPositions[arrayIdx].size() <= elementIdx)
    {
        return false;
    }
    
    // The view has already binded, early out
    if (m_cbvSrvUavDescriptorPositions[arrayIdx][elementIdx] != INT_MAX)
    {
        CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHeapHandle(m_cbvSrvUavRingBuffer->GetGPUDescriptorHandleForHeapStart(), m_cbvSrvUavDescriptorPositions[arrayIdx][elementIdx], m_cbvSrvUavDescriptorSize);
        outGpuDescriptorHandle = gpuHeapHandle;

        return true;
    }

    CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHeapHandleSrc(m_cbvSrvUavDescriptorHeaps[arrayIdx]->GetCPUDescriptorHandleForHeapStart(), (INT)elementIdx, m_cbvSrvUavDescriptorSize);
    CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHeapHandleDst(m_cbvSrvUavRingBuffer->GetCPUDescriptorHandleForHeapStart(), m_ringBufferItr, m_cbvSrvUavDescriptorSize);
    m_pDevice->CopyDescriptorsSimple(
        1,
        cpuHeapHandleDst,
        cpuHeapHandleSrc,
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    // Update the non shader visible heap pointer to ring buffer itr
    m_cbvSrvUavDescriptorPositions[arrayIdx][elementIdx] = m_ringBufferItr;

    if (m_cbvSrvUavRingBufferIds[m_ringBufferItr] != UINT64_MAX)
    {
        // Clear the position of corresponding non shader visible heap if needed
        GetArrayIdxAndElementIdxByViewId(m_cbvSrvUavRingBufferIds[m_ringBufferItr], arrayIdx, elementIdx);
        m_cbvSrvUavDescriptorPositions[arrayIdx][elementIdx] = INT_MAX;
    }

    // Update viewId of ring buffer pos
    m_cbvSrvUavRingBufferIds[m_ringBufferItr] = viewId;

    CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHeapHandle(m_cbvSrvUavRingBuffer->GetGPUDescriptorHandleForHeapStart(), m_ringBufferItr, m_cbvSrvUavDescriptorSize);
    outGpuDescriptorHandle = gpuHeapHandle;

    // advance the iterator
    m_ringBufferItr = (m_ringBufferItr + 1) % m_defaultRingBufferSize;

    return true;
}

bool DescriptorHeapManager::GetRenderTargetViewCpuHandle(uint64_t viewId, D3D12_CPU_DESCRIPTOR_HANDLE& outCpuDescriptorHandle)
{
    if (viewId >= m_currentRtvCount)
    {
        return false;
    }

    CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHeapHandle(m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), (INT)viewId, m_rtvDescriptorSize);
    outCpuDescriptorHandle = cpuHeapHandle;
    
    return true;
}

bool DescriptorHeapManager::GetDepthStencilViewCpuHandle(uint64_t viewId, D3D12_CPU_DESCRIPTOR_HANDLE& outCpuDescriptorHandle)
{
    if (viewId >= m_currentDsvCount)
    {
        return false;
    }

    CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHeapHandle(m_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), (INT)viewId, m_dsvDescriptorSize);
    outCpuDescriptorHandle = cpuHeapHandle;

    return true;
}

bool DescriptorHeapManager::GetSampler(uint64_t viewId, D3D12_GPU_DESCRIPTOR_HANDLE& outGpuDescriptorHandle)
{
    if (viewId >= m_currentSamplerCount)
    {
        return false;
    }

    CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHeapHandle(m_samplerDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), (INT)viewId, m_samplerDescriptorSize);
    outGpuDescriptorHandle = gpuHeapHandle;

    return true;
}

DescriptorHeapManager::~DescriptorHeapManager()
{
    if (m_rtvDescriptorHeap)
    {
        m_rtvDescriptorHeap.Reset();
    }

    if (m_dsvDescriptorHeap)
    {
        m_dsvDescriptorHeap.Reset();
    }

    if (m_samplerDescriptorHeap)
    {
        m_samplerDescriptorHeap.Reset();
    }

    if (m_cbvSrvUavRingBuffer)
    {
        m_cbvSrvUavRingBuffer.Reset();
    }

    for (auto& descHeap : m_cbvSrvUavDescriptorHeaps)
    {
        if (descHeap)
        {
            descHeap.Reset();
        }
    }

    m_cbvSrvUavDescriptorHeaps.clear();
    m_cbvSrvUavDescriptorPositions.clear();
    m_cbvSrvUavRingBufferIds.clear();
}