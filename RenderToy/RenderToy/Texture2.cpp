#include "Texture2.h"
#include <fstream>

Texture2::Texture2(std::string name)
	: m_name(name)
{
}

Texture2::Texture2(std::string name, std::filesystem::path filePath, uint32_t fileOffset, uint32_t dataSize, uint32_t width, uint32_t height, uint32_t numChannels)
	: m_name(name), m_filePath(filePath), m_fileOffset(fileOffset), m_dataSize(dataSize), m_width(width), m_height(height), m_numChannels(numChannels)
{
}

Texture2::~Texture2()
{
}

uint32_t Texture2::AddStreamingReference()
{
	return ++m_streamingReferenceCount;
}

uint32_t Texture2::ReleaseStreamingReference()
{
	return --m_streamingReferenceCount;
}

bool Texture2::StreamIn(GraphicsContext* graphicsContext, CommandBuilder* cmdBuilder)
{
	if (m_streamedIn)
	{
		return true;
	}

	if (m_streamingReferenceCount == 0)
	{
		return false;
	}

	std::fstream file(m_filePath, std::ios::in | std::ios::binary);
	if (!file.is_open())
	{
		return false;
	}

	char* data = new char[m_dataSize]();
	file.seekg(m_fileOffset);
	file.read((char*)data, m_dataSize); // TODO: read actual data

	D3D12_RESOURCE_DESC texDesc = {};
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	texDesc.Width = m_width;
	texDesc.Height = m_height;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = m_numChannels == 1 ? DXGI_FORMAT_R8_UNORM : DXGI_FORMAT_B8G8R8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	m_resource = std::make_unique<D3DResource>(true);
	m_resource->Initialize(graphicsContext, &texDesc, data, m_dataSize, texDesc.Width * m_numChannels);
	cmdBuilder->TryReset();
	m_resource->CopyToDefaultHeap(cmdBuilder->GetCommandList());

	ID3D12Resource* defaultResource = m_resource->GetDefaultResource();
	DescriptorHeapManager* descHeapManager = graphicsContext->GetDescriptorHeapManager();
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.PlaneSlice = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	m_srvId = descHeapManager->CreateShaderResourceView(defaultResource, &srvDesc);

	file.close();

	delete[] data;

	return true;
}