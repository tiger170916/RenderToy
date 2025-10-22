#pragma once

#include "Includes.h"
#include "D3DResource.h"
#include "GraphicsContext.h"

class Texture2
{
friend class TextureManager2;
private:
	std::string m_name;

	std::unique_ptr<D3DResource> m_resource = nullptr;

	uint32_t m_streamingReferenceCount = 0;

	std::filesystem::path m_filePath;

	// Metadata
	uint32_t m_fileOffset = 0;

	uint32_t m_dataSize = 0;

	uint32_t m_width = 0;

	uint32_t m_height = 0;

	uint32_t m_numChannels = 0;

	uint64_t m_srvId = UINT64_MAX;

	bool m_streamedIn = false;


public:
	Texture2(std::string name);

	Texture2(std::string name, std::filesystem::path filePath, uint32_t fileOffset, uint32_t dataSize, uint32_t width, uint32_t height, uint32_t numChannels);

	uint32_t AddStreamingReference();

	uint32_t ReleaseStreamingReference();

	uint32_t GetStreamingReferenceCount() const { return m_streamingReferenceCount; }

	bool StreamIn(GraphicsContext* graphicsContext, CommandBuilder* cmdBuilder);

	uint64_t GetSrvId() const { return m_srvId; }

	bool IsStreamedIn() const { return m_streamedIn; }

protected:
	void SetStreamedIn(bool streamedIn) { m_streamedIn = streamedIn; }


public:
	~Texture2();
};