#include "IMesh.h"
#include "UidGenerator.h"
#include "Macros.h"

std::map<PassType, std::map<UINT, UINT>> IMesh::_passMeshArgumentsMap = PASS_MESH_ARGUMENTS_MAP_DEFINE;

IMesh::IMesh(std::string meshName)
	: m_meshName(meshName)
{
	m_uid = UidGenerator::Get()->GenerateUid();
}

IMesh::~IMesh()
{

}

void IMesh::AddFileMetadataPart(UINT offset, UINT size, std::string materialName)
{
	m_meshPartDataOffsets.push_back(offset);
	m_meshPartDataSizes.push_back(size);
	m_MeshPartMaterialNames.push_back(materialName);
}

void IMesh::SetResourceFilePath(std::filesystem::path filePath)
{
	m_isFromeFile = true;
	m_filePath = filePath;
}

/// <summary>
/// Load the mesh from file
/// </summary>
bool IMesh::LoadFromBinary(
	GraphicsContext* graphicsContext,
	MaterialManager* materialManager,
	TextureManager2* texManager,
	CommandBuilder* cmdBuilder)
{
	if (m_isUploadedToGpu)
	{
		return true;
	}

	if (!graphicsContext)
	{
		return false;
	}

	std::fstream meshFile(m_filePath, std::ios::in | std::ios::binary);
	if (!meshFile.is_open())
	{
		return false;
	}

	for (size_t partIdx = 0; partIdx < m_meshPartDataOffsets.size(); partIdx++)
	{
		cmdBuilder->TryReset();
		meshFile.seekg(m_meshPartDataOffsets[partIdx]);
		byte* partData = new byte[m_meshPartDataSizes[partIdx]]();
		meshFile.read((char*)partData, m_meshPartDataSizes[partIdx]);

		std::unique_ptr<MeshPartStruct> meshPart = std::make_unique<MeshPartStruct>();
		meshPart->bufferSize = m_meshPartDataSizes[partIdx];
		meshPart->vertexCount = m_meshPartDataSizes[partIdx] / sizeof(MeshVertexDx);

		auto vbDesc = CD3DX12_RESOURCE_DESC::Buffer(meshPart->bufferSize);
		meshPart->pResource = std::make_unique<D3DResource>(true);
		meshPart->pResource->Initialize(graphicsContext, &vbDesc, partData, meshPart->bufferSize, 0);
		meshPart->pResource->CopyToDefaultHeap(cmdBuilder->GetCommandList());
		meshPart->vertexBufferView.BufferLocation = meshPart->pResource->GetDefaultResource()->GetGPUVirtualAddress();
		meshPart->vertexBufferView.SizeInBytes = meshPart->bufferSize;
		meshPart->vertexBufferView.StrideInBytes = sizeof(MeshVertexDx);
		meshPart->materialName = m_MeshPartMaterialNames[partIdx];

		Material2* material = materialManager->GetMaterial(meshPart->materialName);
		if (material)
		{
			material->StreamIn(texManager);
		}

		m_meshParts.push_back(std::move(meshPart));

		delete[] partData;
	}

	//model cb
	m_instanceConstants = std::unique_ptr< ConstantBuffer<MeshInstanceConstants>>(new ConstantBuffer<MeshInstanceConstants>(64));
	if (!m_instanceConstants->Initialize(graphicsContext))
	{
		return false;
	}

	m_isUploadedToGpu = true;
	return true;
}