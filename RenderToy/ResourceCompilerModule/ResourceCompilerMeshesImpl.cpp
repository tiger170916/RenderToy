#include "ResourceCompilerMeshesImpl.h"

MaterialImpl::MaterialImpl(std::string name)
	: m_materialName(name)
{
}

MeshPartImpl::MeshPartImpl(const uint32_t& dataOffset, const uint32_t& dataSize, const uint32_t& numVertices)
	: m_dataOffset(dataOffset), m_dataSize(dataSize), m_numVertices(numVertices)
{
}

void MeshImpl::GetMeshParts(std::vector<MeshPart*>& outMeshParts)
{
	outMeshParts.clear();

	for (size_t i = 0; i < m_meshParts.size(); i++)
	{
		outMeshParts.push_back(m_meshParts[i].get());
	}
}

void MeshesDataImpl::GetMeshes(std::vector<Mesh*>& outMeshes)
{
	outMeshes.clear();

	for (size_t i = 0; i < m_meshes.size(); i++)
	{
		outMeshes.push_back(m_meshes[i].get());
	}
}

MeshImpl::MeshImpl(std::string name)
	: m_name(name){}