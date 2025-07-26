#include "Texture.h"

std::map<GUID, DXGI_FORMAT, GuidComparator> Texture::_pixelFormatLookup =
{
	{GUID_WICPixelFormat32bppBGRA, DXGI_FORMAT_B8G8R8A8_UNORM},
	//{GUID_WICPixelFormat32bppRGBA, DXGI_FORMAT_R8G8B8A8_UNORM},
	{GUID_WICPixelFormat24bppBGR, DXGI_FORMAT_B8G8R8A8_UNORM},
	//{GUID_WICPixelFormat24bppRGB, DXGI_FORMAT_R8G8B8A8_UNORM},
	//{GUID_WICPixelFormat16bppGray, DXGI_FORMAT_R8G8B8A8_UNORM},
	//{GUID_WICPixelFormat48bppBGR, DXGI_FORMAT_R8G8B8A8_UNORM},
	//{GUID_WICPixelFormat48bppRGB, DXGI_FORMAT_R8G8B8A8_UNORM},
	{GUID_WICPixelFormat8bppGray, DXGI_FORMAT_R8_UNORM},
};

Texture::Texture(std::filesystem::path path):
	m_path(path) 
{

}

Texture::~Texture() {}

bool Texture::StreamIn(GraphicsContext* graphicsContext)
{
	if (m_inUploadHeapMemory)
	{
		return true;
	}

	if (!graphicsContext)
	{
		return false;
	}

	// Create image factory.
	ComPtr<IWICImagingFactory> wicfactory;
	CoInitialize(nullptr);

	if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wicfactory))))
	{
		DWORD err = GetLastError();
		return false;
	}

	ComPtr<IWICStream> fs;
	if (FAILED(wicfactory->CreateStream(&fs)))
	{
		return false;
	}

	// Load image.
	std::string filepathStr = m_path.string();
	if (FAILED(fs->InitializeFromFilename(std::wstring(filepathStr.begin(), filepathStr.end()).c_str(), GENERIC_READ)))
	{
		return false;
	}

	// Initialize decoder
	ComPtr<IWICBitmapDecoder> decoder;
	if (FAILED(wicfactory->CreateDecoderFromStream(fs.Get(), nullptr, WICDecodeMetadataCacheOnLoad, &decoder)))
	{
		return false;
	}

	ComPtr<IWICBitmapFrameDecode> frameDecoder;
	if (FAILED(decoder->GetFrame(0, &frameDecoder)))
	{
		return false;
	}

	// Get image info
	if (FAILED(frameDecoder->GetSize(&m_width, &m_height)))
	{
		return false;
	}

	if (FAILED(frameDecoder->GetPixelFormat(&m_pixelFormat)))
	{
		return false;
	}

	ComPtr<IWICComponentInfo> compInfo;
	if (FAILED(wicfactory->CreateComponentInfo(m_pixelFormat, &compInfo)))
	{
		return false;
	}

	ComPtr<IWICPixelFormatInfo> pixelFormatInfo;
	if (FAILED(compInfo->QueryInterface(__uuidof(IWICPixelFormatInfo), &pixelFormatInfo)))
	{
		return false;
	}

	// Not supported pixel format
	if (!_pixelFormatLookup.contains(m_pixelFormat))
	{
		return false;
	}

	m_dxgiFormat = _pixelFormatLookup[m_pixelFormat];

	pixelFormatInfo->GetBitsPerPixel(&m_bitsPerPixel);
	pixelFormatInfo->GetChannelCount(&m_numChannels);

	m_stride = (m_bitsPerPixel / 8) * m_width;
	m_dataSize = m_stride * m_height;
	byte* data = (byte*)malloc(m_dataSize);

	if (!data)
	{
		return false;
	}
	memset(data, 0, m_dataSize);
	WICRect rect
	{
		.X = 0,
		.Y = 0,
		.Width = static_cast<INT>(m_width),
		.Height = static_cast<INT>(m_height)
	};

	if (FAILED(frameDecoder->CopyPixels(&rect, m_stride, m_dataSize, data)))
	{
		free(data);
		return false;
	}

	m_resource = std::unique_ptr<D3DResource>(new D3DResource(true));

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	resourceDesc.Width = m_width;
	resourceDesc.Height = m_height;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = m_dxgiFormat;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	// There is no dxgi format with 3 channels. so have to create new buffer which contains 4 channels
	if (m_numChannels == 3)
	{
		byte* tmpData = new byte[m_width * m_height * 4];
		if (!tmpData)
		{
			return false;
		}

		uint32_t itrOrigin = 0;
		uint32_t itrTmp = 0;
		for (uint32_t i = 0; i < m_width * m_height; i++)
		{
			tmpData[itrTmp] = data[itrOrigin];
			tmpData[itrTmp + 1] = data[itrOrigin + 1];
			tmpData[itrTmp + 2] = data[itrOrigin + 2];
			itrOrigin += 3;
			itrTmp += 4;
		}

		free(data);
		data = tmpData;
		m_stride = m_width * 4;
		m_dataSize = m_stride * m_height;
	}

	if (!m_resource->Initialize(graphicsContext, &resourceDesc, data, (UINT)m_dataSize, m_stride))
	{
		free(data);
		return false;
	}

	free(data);

	// Create shader resource view.
	D3D12_SHADER_RESOURCE_VIEW_DESC srv{};
	srv.Format = m_dxgiFormat;
	srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srv.Texture2D.MipLevels = 1;
	srv.Texture2D.MostDetailedMip = 0;
	srv.Texture2D.PlaneSlice = 0;
	srv.Texture2D.ResourceMinLODClamp = 0.0f;

	DescriptorHeapManager* descHeapManager = graphicsContext->GetDescriptorHeapManager();
	m_srvId = descHeapManager->CreateShaderResourceView(m_resource->GetDefaultResource(), &srv);

	m_inUploadHeapMemory = true;

	/*
	m_elementSize = (m_bitsPerPixel + 7) / 8;
	// Image parsing
	m_stride = m_elementSize * m_width;
	UINT originalDataSize = m_stride * m_height;
	byte* originalData = (byte*)malloc(originalDataSize);

	if (!originalData)
	{
		return false;
	}

	memset(originalData, 0, originalDataSize);

	WICRect rect
	{
		.X = 0,
		.Y = 0,
		.Width = static_cast<INT>(m_width),
		.Height = static_cast<INT>(m_height)
	};

	if (FAILED(frameDecoder->CopyPixels(&rect, m_stride, originalDataSize, originalData)))
	{
		free(originalData);
		return false;
	}

	int actualElementSize = 4;
	int bitsPerChannel = m_bitsPerPixel / m_numChannels;

	m_dataSize = m_width * m_height * actualElementSize;

	bool needCreateNewBuffer = !(m_numChannels == 4 && bitsPerChannel == 8);

	byte* data = needCreateNewBuffer ? new byte[m_dataSize] : nullptr;

	if (needCreateNewBuffer)
	{
		memset(data, 0, m_dataSize);
	}

	if (m_numChannels == 4)
	{
		if (bitsPerChannel == 8)
		{
			//memcpy(data, originalData, m_dataSize);
			// no need to convert
			data = originalData;
		}
		else if (bitsPerChannel == 16)
		{
			byte* dstBufferItr = data;
			uint16_t* srcBufferItr = (uint16_t*)originalData;

			for (uint32_t i = 0; i < m_width * m_height; i++)
			{
				for (int comp = 0; comp < 4; comp++)
				{
					uint8_t compVal;
					Convert16BitsTo8Bits(srcBufferItr, compVal);
					*data = compVal;
					dstBufferItr += 1;
					srcBufferItr += 1;
				}
			}
		}
	}
	else if (m_numChannels == 3)
	{
		byte* dstBufferItr = data;

		if (bitsPerChannel == 8)
		{
			byte* srcBufferItr = originalData;
			for (uint32_t i = 0; i < m_width * m_height; i++)
			{
				// Copy the current rgb/gbr data.
				memcpy(dstBufferItr, srcBufferItr, 3);
				// Insert default alpha data
				*(dstBufferItr + 3) = 0xFF;
				// Advance pointers.
				srcBufferItr += 3;
				dstBufferItr += 4;
			}
		}
		else if (bitsPerChannel == 16)
		{
			uint16_t* srcBufferItr = (uint16_t*)originalData;
			for (uint32_t i = 0; i < m_width * m_height; i++)
			{
				for (int comp = 0; comp < 3; comp++)
				{
					uint8_t compVal;
					Convert16BitsTo8Bits(srcBufferItr, compVal);
					*dstBufferItr = compVal;
					srcBufferItr += 1;
					dstBufferItr += 1;
				}

				*dstBufferItr = 0xFF;
				dstBufferItr += 1;
			}
		}
	}
	else if (m_numChannels == 1)
	{
		byte* dstBufferItr = data;

		if (bitsPerChannel == 8)
		{
			byte* srcBufferItr = originalData;
			for (uint32_t i = 0; i < m_width * m_height; i++)
			{
				// Copy the current rgb/gbr data.
				*dstBufferItr = *srcBufferItr;
				// Insert default alpha data
				*(dstBufferItr + 1) = 0xFF;
				*(dstBufferItr + 2) = 0xFF;
				*(dstBufferItr + 3) = 0xFF;
				// Advance pointers.
				srcBufferItr += 1;
				dstBufferItr += 4;
			}
		}
		else if (bitsPerChannel == 16)
		{
			uint16_t* srcBufferItr = (uint16_t*)data;
			for (uint32_t i = 0; i < m_width * m_height; i++)
			{
				// Copy the current rgb/gbr data.
				uint8_t compVal;
				Convert16BitsTo8Bits(srcBufferItr, compVal);
				*dstBufferItr = compVal;
				// Insert default alpha data
				*(dstBufferItr + 1) = 0xFF;
				*(dstBufferItr + 2) = 0xFF;
				*(dstBufferItr + 3) = 0xFF;
				// Advance pointers.
				srcBufferItr += 1;
				dstBufferItr += 4;
			}
		}
	}

	m_resource = std::unique_ptr<D3DResource>(new D3DResource(true));

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	resourceDesc.Width = m_width;
	resourceDesc.Height = m_height;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = m_dxgiFormat;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	if (!m_resource->Initialize(graphicsContext, &resourceDesc, data, (UINT)m_dataSize))
	{
		free(data);
		return false;
	}

	free(data);
	if (needCreateNewBuffer)
	{
		free(originalData);
	}

	// Create shader resource view.
	D3D12_SHADER_RESOURCE_VIEW_DESC srv{};
	srv.Format = m_dxgiFormat;
	srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srv.Texture2D.MipLevels = 1;
	srv.Texture2D.MostDetailedMip = 0;
	srv.Texture2D.PlaneSlice = 0;
	srv.Texture2D.ResourceMinLODClamp = 0.0f;

	DescriptorHeapManager* descHeapManager = graphicsContext->GetDescriptorHeapManager();
	m_srvId = descHeapManager->CreateShaderResourceView(m_resource->GetDefaultResource(), &srv);

	m_inUploadHeapMemory = true;
	*/
	return true;
}

bool Texture::ScheduleForCopyToDefaultHeap(ID3D12GraphicsCommandList* cmdList)
{
	// Have to make sure the buffer is in upload heap memory, before go ahead and schedule a copy command.
	if (!m_inUploadHeapMemory)
	{
		return false;
	}

	return m_resource->CopyToDefaultHeap(cmdList);
}

bool Texture::StreamOut()
{
	return false;
}

bool Texture::BindShaderResourceViewToPipeline(GraphicsContext* graphicsContext, D3D12_GPU_DESCRIPTOR_HANDLE& outGpuDescHandle)
{
	if (!graphicsContext)
	{
		return false;
	}

	DescriptorHeapManager* descriptorHeapManager = graphicsContext->GetDescriptorHeapManager();

	return descriptorHeapManager->BindCbvSrvUavToPipeline(m_srvId, outGpuDescHandle);
}