#include "Textures.h"
#include "TextureLoader.h"

void Textures::AddTexture(
	std::string name,
	std::string file)
{
	TextureDefine textureDefine = {};
	textureDefine.Name = name;
	textureDefine.File = file;
	m_textures.push_back(textureDefine);
}

bool Textures::PackToBinary(std::filesystem::path rootFilePath)
{
	std::filesystem::path outputDir = rootFilePath.parent_path().append(rootFilePath.stem().string());
	std::filesystem::create_directory(outputDir);

	std::filesystem::path outputFile = outputDir;
	outputFile.append(rootFilePath.stem().string()).concat(".bin");

	std::ofstream file(outputFile, std::ios::out | std::ios::binary);
	if (!file.is_open()) {
		return false;
	}

	std::filesystem::path rootPath = rootFilePath.parent_path();

	std::byte headerBytes[1024] = { };
	// Write binary header
	BinaryHeader binaryHeader = {};
	binaryHeader.BinaryType = BinaryType::BINARY_TYPE_TEXTURES;
	binaryHeader.MagicNum = 0x12345678;   // 0x12345678 - Magic number  (4 bytes)
	binaryHeader.Version = 0x00000001;	// 0x00000001 - Version       (4 bytes)

	uint32_t dataOffset = binaryHeader.TexturesOffset + (uint32_t)m_textures.size() * sizeof(TextureHeader);

	std::vector<TextureHeader> allTextureHeaders;
	for (size_t i = 0; i < m_textures.size(); i++)
	{
		std::filesystem::path assetFilePath = rootPath;
		assetFilePath.append(m_textures[i].File);

		uint32_t width;
		uint32_t height;
		uint32_t fileSize;
		uint32_t numChannels;
		uint32_t offset = dataOffset;
		TextureLoader textureLoader;
		file.seekp(offset);
		if (textureLoader.LoadTexture(
			assetFilePath,
			&file,
			&dataOffset,
			width,
			height,
			fileSize,
			numChannels))
		{
			TextureHeader texHeader = {};
			memcpy(texHeader.TextureName, m_textures[i].Name.c_str(), m_textures[i].Name.size());
			texHeader.NumChannels = numChannels;
			texHeader.TextureDataSize = fileSize;
			texHeader.TextureOffset = offset;
			texHeader.TextureWidth = width;
			texHeader.TextureHeight = height;

			allTextureHeaders.push_back(texHeader);
		}
	}

	binaryHeader.NumTextures = (uint32_t)allTextureHeaders.size();
	binaryHeader.TexturesOffset = (uint32_t)1024;

	memcpy(headerBytes, &binaryHeader, sizeof(BinaryHeader));

	file.seekp(0);
	file.write((char*)headerBytes, 1024);	// Write header
	file.write((char*)allTextureHeaders.data(), allTextureHeaders.size() * sizeof(TextureHeader)); // Write texture headers

	file.close();

	return true;
}