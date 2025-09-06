#include "TextureLoader.h"
std::map<GUID, uint32_t, GuidComparator> TextureLoader::_pixelFormatLookup =
{
	{GUID_WICPixelFormat32bppBGRA, 4},
	//{GUID_WICPixelFormat32bppRGBA, DXGI_FORMAT_R8G8B8A8_UNORM},
	{GUID_WICPixelFormat24bppBGR, 4},
	//{GUID_WICPixelFormat24bppRGB, DXGI_FORMAT_R8G8B8A8_UNORM},
	//{GUID_WICPixelFormat16bppGray, DXGI_FORMAT_R8G8B8A8_UNORM},
	//{GUID_WICPixelFormat48bppBGR, DXGI_FORMAT_R8G8B8A8_UNORM},
	//{GUID_WICPixelFormat48bppRGB, DXGI_FORMAT_R8G8B8A8_UNORM},
	{GUID_WICPixelFormat8bppGray, 1},
};

bool TextureLoader::LoadTexture(
	std::filesystem::path filePath,
	std::ofstream* file,
	uint32_t* textureOffset,
	uint32_t& outWidth,
	uint32_t& outHeight,
	uint32_t& outFileSize,
	uint32_t& outNumChannels)
{
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
	std::string filepathStr = filePath.string();
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

	UINT width;
	UINT height;
	WICPixelFormatGUID wicPixelFormat;
	// Get image info
	if (FAILED(frameDecoder->GetSize(&width, &height)))
	{
		return false;
	}

	if (FAILED(frameDecoder->GetPixelFormat(&wicPixelFormat)))
	{
		return false;
	}

	ComPtr<IWICComponentInfo> compInfo;
	if (FAILED(wicfactory->CreateComponentInfo(wicPixelFormat, &compInfo)))
	{
		return false;
	}

	ComPtr<IWICPixelFormatInfo> pixelFormatInfo;
	if (FAILED(compInfo->QueryInterface(__uuidof(IWICPixelFormatInfo), &pixelFormatInfo)))
	{
		return false;
	}

	// Not supported pixel format
	if (!_pixelFormatLookup.contains(wicPixelFormat))
	{
		return false;
	}

	uint32_t numChannels = 0;//_pixelFormatLookup[wicPixelFormat];
	uint32_t numBitsPerPixel = 0;
	uint32_t stride = 0;
	
	pixelFormatInfo->GetBitsPerPixel(&numBitsPerPixel);
	pixelFormatInfo->GetChannelCount(&numChannels);

	stride = (numBitsPerPixel / 8) * width;
	uint32_t dataSize = stride * height;
	byte* data = (byte*)malloc(dataSize);

	if (!data)
	{
		return false;
	}

	memset(data, 0, dataSize);
	WICRect rect
	{
		.X = 0,
		.Y = 0,
		.Width = static_cast<INT>(width),
		.Height = static_cast<INT>(height)
	};

	if (FAILED(frameDecoder->CopyPixels(&rect, stride, dataSize, data)))
	{
		free(data);
		return false;
	}

	// There is no dxgi format with 3 channels. so have to create new buffer which contains 4 channels
	if (
		numChannels == 3)
	{
		byte* tmpData = new byte[width * height * 4];
		if (!tmpData)
		{
			return false;
		}

		uint32_t itrOrigin = 0;
		uint32_t itrTmp = 0;
		for (uint32_t i = 0; i < width * height; i++)
		{
			tmpData[itrTmp] = data[itrOrigin];
			tmpData[itrTmp + 1] = data[itrOrigin + 1];
			tmpData[itrTmp + 2] = data[itrOrigin + 2];
			itrOrigin += 3;
			itrTmp += 4;
		}

		free(data);
		data = tmpData;
		stride = width * 4;
		dataSize = stride * height;
		numChannels = 4;
	}

	file->write((char*)data, dataSize);
	*textureOffset += dataSize;
	outWidth = width;
	outHeight = height;
	outFileSize = dataSize;
	outNumChannels = numChannels;

	free(data);
	return true;
}