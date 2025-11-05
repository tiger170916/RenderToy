#include "ResourceCompilerMeshesImpl.h"

MaterialImpl::MaterialImpl(std::string name)
	: m_materialName(name)
{
}

MeshPartImpl::MeshPartImpl(const uint32_t& dataOffset, const uint32_t& dataSize, const uint32_t& numVertices)
	: m_dataOffset(dataOffset), m_dataSize(dataSize), m_numVertices(numVertices)
{
}

void MeshImpl::GetMeshParts(MeshPart** ppMeshParts, uint32_t* pNumParts)
{
	if (!ppMeshParts || !pNumParts)
	{
		return;
	}

	if (m_meshParts.empty())
	{
		*ppMeshParts = nullptr;
		*pNumParts = 0;
		return;
	}

	*ppMeshParts = (MeshPart*) malloc(sizeof(void*) * m_meshParts.size());
	for (size_t i = 0; i < m_meshParts.size(); i++)
	{
		ppMeshParts[i] = m_meshParts[i].get();
	}
}

void MeshesDataImpl::GetMeshes(Mesh** ppMeshes, uint32_t* pNumMeshes)
{
	if (!ppMeshes || !pNumMeshes)
	{
		return;
	}

	if (m_meshes.empty())
	{
		*ppMeshes = nullptr;
		*pNumMeshes = 0;
		return;
	}

	*ppMeshes = (Mesh*)malloc(sizeof(void*) * m_meshes.size());
	for (size_t i = 0; i < m_meshes.size(); i++)
	{
		ppMeshes[i] = m_meshes[i].get();
	}
}

MeshImpl::MeshImpl(std::string name)
	: m_name(name){}