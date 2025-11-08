#pragma once
#include <string>
#include <vector>

namespace ResourceCompilerModule
{
	class Material
	{
	public:
		virtual std::string GetMaterialName() = 0;

		virtual std::string GetBaseColorTextureName() = 0;
		virtual std::string GetBaseColorTextureFile() = 0;

		virtual std::string GetMetallicTextureName() = 0;
		virtual std::string GetMetallicTextureFile() = 0;

		virtual std::string GetRoughnessTextureName() = 0;
		virtual std::string GetRoughnessTextureFile() = 0;

		virtual std::string GetNormalTextureName() = 0;
		virtual std::string GetNormalTextureFile() = 0;
	};

	class MeshPart
	{
	public:
		virtual uint32_t GetDataOffset() = 0;
		virtual uint32_t GetDataSize() = 0;
		virtual uint32_t GetNumVertices() = 0;
		virtual Material* GetMaterial() = 0;
	};

	class Mesh
	{
	public:
		virtual std::string GetName() = 0;

		virtual void GetMeshParts(std::vector<MeshPart*>& outMeshParts) = 0;
	};

	class MeshesData
	{
	public:
		virtual void GetMeshes(std::vector<Mesh*>& outMeshes) = 0;
	};
}