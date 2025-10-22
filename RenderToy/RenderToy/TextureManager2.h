#pragma once

#include "Includes.h"
#include "Texture2.h"
#include "IStreamable.h"

class TextureManager2 : public IStreamable
{
friend class StreamingEngine;
private:
	std::map<std::string, std::unique_ptr<Texture2>> m_textures;
public:
	TextureManager2();

	~TextureManager2();

	Texture2* GetTexture(std::string name);

	Texture2* GetOrCreateTexture(std::string name, std::filesystem::path filePath, uint32_t fileOffset, uint32_t dataSize, uint32_t width, uint32_t height, uint32_t numChannels);

public:
	virtual bool StreamInBinary(GraphicsContext* graphicsContext, CommandBuilder* cmdBuilder) override;

	virtual bool StreamOut() override;

	virtual bool CleanUpAfterStreamIn()  override;

protected:
	void SetStreamedInDone();
};