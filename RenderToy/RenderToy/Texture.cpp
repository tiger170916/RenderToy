#include "Texture.h"

std::map<GUID, DXGI_FORMAT, GuidComparator> Texture::_pixelFormatLookup =
{
	{GUID_WICPixelFormat32bppBGRA, DXGI_FORMAT_B8G8R8A8_UNORM},
	{GUID_WICPixelFormat32bppRGBA, DXGI_FORMAT_R8G8B8A8_UNORM},
	{GUID_WICPixelFormat24bppBGR, DXGI_FORMAT_B8G8R8A8_UNORM},
	{GUID_WICPixelFormat24bppRGB, DXGI_FORMAT_R8G8B8A8_UNORM},
	{GUID_WICPixelFormat16bppGray, DXGI_FORMAT_R8G8B8A8_UNORM},
	{GUID_WICPixelFormat48bppBGR, DXGI_FORMAT_R8G8B8A8_UNORM},
	{GUID_WICPixelFormat48bppRGB, DXGI_FORMAT_R8G8B8A8_UNORM}
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
	if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wicfactory))))
	{
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

	m_elementSize = (m_bitsPerPixel + 7) / 8;
	// Image parsing
	m_stride = m_elementSize * m_width;
	UINT originalDataSize = m_stride * m_height;
	byte* originalData = (byte*)malloc(originalDataSize);

	if (!originalData)
	{
		return false;
	}

	memset(originalData, m_dataSize, 0);

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

	m_dataSize = m_width * m_height * actualElementSize;
	byte* data = (m_numChannels == 4 && m_bitsPerPixel == 8) ? nullptr : new byte[m_dataSize];
	memset(data, 0, m_dataSize);

	if (m_numChannels == 4)
	{
		if (m_bitsPerPixel == 8)
		{
			//memcpy(data, originalData, m_dataSize);
			// no need to convert
			data = originalData;
		}
		else if (m_bitsPerPixel == 16)
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

		if (m_bitsPerPixel == 8)
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
		else if (m_bitsPerPixel == 16)
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

		if (m_bitsPerPixel == 8)
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
		else if (m_bitsPerPixel == 16)
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

	free(originalData);

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

	return false;
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