#pragma once

#include "Includes.h"
#include "GuidComparator.h"
#include "GraphicsContext.h"
#include "D3DResource.h"
#include "StreamInterface.h"
#include <wincodec.h>
#include <guiddef.h>

class Texture : public StreamInterface
{
private:
	static std::map<GUID, DXGI_FORMAT, GuidComparator> _pixelFormatLookup;

	std::filesystem::path m_path;

	uint32_t m_width = 0;
	uint32_t m_height = 0;
	uint32_t m_elementSize = 0;
	uint32_t m_stride = 0;
	uint32_t m_dataSize = 0;
	uint32_t m_bitsPerPixel = 0;
	uint32_t m_numChannels = 0;

	GUID m_pixelFormat;
	DXGI_FORMAT m_dxgiFormat;

	std::unique_ptr<D3DResource> m_resource = nullptr;

	uint64_t m_srvId = UINT64_MAX;

public:
	Texture(std::filesystem::path path);

	~Texture();

public:
	// Interface implementation
	virtual bool StreamIn(GraphicsContext* graphicsContext) override;

	virtual bool StreamOut() override;

	virtual bool ScheduleForCopyToDefaultHeap(ID3D12GraphicsCommandList* cmdList) override;

private:
	inline void Convert16BitsTo8Bits(uint16_t* in, uint8_t& out){out = *in / 256;}
};