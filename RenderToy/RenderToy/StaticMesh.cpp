#include "StaticMesh.h"
#include "Macros.h"

std::map<PassType, std::map<UINT, UINT>> StaticMesh::_passMeshArgumentsMap = PASS_MESH_ARGUMENTS_MAP_DEFINE;

StaticMesh::StaticMesh(uint32_t uid)
	: m_uid(uid)
{
}

StaticMesh::~StaticMesh()
{
	m_meshParts.clear();
	m_instances.clear();
}

void StaticMesh::AddMeshPart(uint32_t vertexOffset, uint32_t vertexCount, uint32_t bufferSize)
{
	MeshPartStruct* meshPart = new MeshPartStruct();
	meshPart->vertexCount = vertexCount;
	meshPart->verticesOffset = vertexOffset;
	meshPart->bufferSize = bufferSize;

	m_meshParts.push_back(std::unique_ptr<MeshPartStruct>(meshPart));
}

void StaticMesh::AddInstance(const Transform& transform, uint32_t uid)
{
	MeshInstanceStruct* instanceStruct = new MeshInstanceStruct();
	instanceStruct->transform = transform;
	instanceStruct->uid = uid;

	m_instances.push_back(std::unique_ptr<MeshInstanceStruct>(instanceStruct));
}

bool StaticMesh::StaticMeshAddLightExtension(
	uint32_t instanceIdx,
	LightExtension* lightExtension,
	uint32_t uid)
{
	if (instanceIdx >= m_instances.size())
	{
		return false;
	}

	m_instances[instanceIdx]->extensions.push_back(std::unique_ptr<ObjectExtension>(lightExtension));
	return true;
}


void StaticMesh::AddMaterial(Material* material)
{
	m_materials.push_back(std::unique_ptr<Material>(material));
}

void StaticMesh::AddTriangle(const int& part, const MeshVertex& v1, const MeshVertex& v2, const MeshVertex& v3)
{
	/*
	if (!m_meshParts.contains(part))
	{
		m_meshParts[part] = std::vector<MeshVertex>();
	}

	m_meshParts[part].push_back(v1);
	m_meshParts[part].push_back(v2);
	m_meshParts[part].push_back(v3);*/
}

bool StaticMesh::GetInstanceTransform(uint32_t instanceIdx, Transform& outTransform)
{
	/*
	if (instanceIdx >= m_instances.size())
	{
		return false;
	}

	outTransform = m_instances[instanceIdx]->transform;*/
	return true;
}

bool StaticMesh::GetInstanceUid(uint32_t instanceIdx, uint32_t& outUid)
{
	if (instanceIdx >= m_instances.size())
	{
		return false;
	}

	outUid = m_instances[instanceIdx]->uid;
	return true;
}

bool StaticMesh::HasLightExtensions() const
{
	/*
	for (const auto& instance : m_instances)
	{
		if (instance->lightExtension != nullptr)
		{
			return true;
		}
	}*/

	return false;
}

const bool StaticMesh::HasLightExtension(const uint32_t& instanceIdx)
{
	/*
	if (instanceIdx >= m_instances.size())
	{
		return false;
	}

	return m_instances[instanceIdx]->lightExtension != nullptr;*/
	return false;
}

LightExtension* StaticMesh::GetLightExtension(const uint32_t& instanceIdx)
{
	/*
	if (instanceIdx >= m_instances.size())
	{
		return nullptr;
	}

	return m_instances[instanceIdx]->lightExtension.get();*/
	return nullptr;
}

void StaticMesh::EnablePass(const PassType& renderPass)
{
	m_enabledPasses.insert(renderPass);
}

bool StaticMesh::PassEnabled(const PassType& renderPass)
{
	return m_enabledPasses.contains(renderPass);
}

bool StaticMesh::BuildResource(GraphicsContext* graphicsContext, TextureManager* textureManager)
{
	/*
	if (!graphicsContext)
	{
		return false;
	}

	ID3D12Device* pDevice = graphicsContext->GetDevice();

	for (auto& pair : m_meshParts)
	{
		const int& materialIdx = pair.first;
		std::vector<MeshVertex>& vertices = pair.second;
		if (vertices.empty())
		{
			continue;
		}

		std::vector<MeshVertexDx> verticesDx;
		std::vector<MeshVertexSimpleDx> verticesSimpleDx;
		for (int i = 0; i < vertices.size(); i++)
		{
			const MeshVertex& vert = vertices[i];
			MeshVertexDx vertDx = {};
			MeshVertexSimpleDx vertSimpleDx = {};
			vertDx.position[0] = vert.Position.X;
			vertDx.position[1] = vert.Position.Y;
			vertDx.position[2] = vert.Position.Z;
			vertDx.UV0[0] = vert.UvSets[0].X;
			vertDx.UV0[1] = vert.UvSets[0].Y;
			vertDx.Normal[0] = vert.Normal.X;
			vertDx.Normal[1] = vert.Normal.Y;
			vertDx.Normal[2] = vert.Normal.Z;

			vertSimpleDx.position[0] = vert.Position.X;
			vertSimpleDx.position[1] = vert.Position.Y;
			vertSimpleDx.position[2] = vert.Position.Z;

			verticesDx.push_back(vertDx);
			verticesSimpleDx.push_back(vertSimpleDx);
		}

		D3DResource* vertexBufferResource = new D3DResource(true);
		D3DResource* vertexSimpleBufferResource = new D3DResource(true);

		// Create vb 
		auto vbDesc = CD3DX12_RESOURCE_DESC::Buffer(verticesDx.size() * sizeof(MeshVertexDx));
		auto vbSimpleDesc = CD3DX12_RESOURCE_DESC::Buffer(verticesSimpleDx.size() * sizeof(MeshVertexSimpleDx));

		if (!vertexBufferResource->Initialize(graphicsContext, &vbDesc, verticesDx.data(), (UINT)verticesDx.size() * sizeof(MeshVertexDx), 0))
		{
			return false;
		}

		if (!vertexSimpleBufferResource->Initialize(graphicsContext, &vbSimpleDesc, verticesSimpleDx.data(), (UINT)verticesSimpleDx.size() * sizeof(MeshVertexSimpleDx), 0))
		{
			return false;
		}

		m_vbResources.push_back(std::unique_ptr<D3DResource>(vertexBufferResource));
		m_simpleVbResources.push_back(std::unique_ptr<D3DResource>(vertexSimpleBufferResource));

		// Create vb view
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
		vertexBufferView.BufferLocation = vertexBufferResource->GetDefaultResource()->GetGPUVirtualAddress();
		vertexBufferView.StrideInBytes = sizeof(MeshVertexDx);
		vertexBufferView.SizeInBytes = (UINT)verticesDx.size() * sizeof(MeshVertexDx);

		D3D12_VERTEX_BUFFER_VIEW vertexSimpleBufferView = {};
		vertexSimpleBufferView.BufferLocation = vertexSimpleBufferResource->GetDefaultResource()->GetGPUVirtualAddress();
		vertexSimpleBufferView.StrideInBytes = sizeof(MeshVertexSimpleDx);
		vertexSimpleBufferView.SizeInBytes = (UINT)verticesSimpleDx.size() * sizeof(MeshVertexSimpleDx);

		m_vertexBufferViews.push_back(vertexBufferView);
		m_simpleVertexBufferViews.push_back(vertexSimpleBufferView);

		m_vertexCounts.push_back((UINT)vertices.size());
	}

	//model cb
	m_instanceConstants = std::unique_ptr< ConstantBuffer<MeshInstanceConstants>>(new ConstantBuffer<MeshInstanceConstants>(64));
	if (!m_instanceConstants->Initialize(graphicsContext))
	{
		return false;
	}

	// Initialize materials
	if (textureManager)
	{
		for (auto& material : m_materials)
		{
			if (!material)
			{
				continue;
			}

			material->Initialize(graphicsContext, textureManager);
		}
	}
	*/
	return true;
}

bool StaticMesh::UpdateBuffers()
{
	/*
	for (int i = 0; i < m_instances.size(); i++)
	{
		MeshInstanceConstants updateCb = {};

		Transform& transform = m_instances[i]->transform;
		XMMATRIX transformMatrix = XMMatrixIdentity();
		XMMATRIX rotation = XMMatrixRotationRollPitchYaw(transform.Rotation.Pitch, transform.Rotation.Yaw, transform.Rotation.Roll);
		XMMATRIX translation = XMMatrixTranslation(transform.Translation.X, transform.Translation.Y, transform.Translation.Z);
		XMMATRIX scale = XMMatrixScaling(transform.Scale.X, transform.Scale.Y, transform.Scale.Z);
		transformMatrix = scale * rotation * translation;

		DirectX::XMMATRIX transformTranspose = DirectX::XMMatrixTranspose(transformMatrix);
		DirectX::XMStoreFloat4x4(&updateCb.TransformMatrix, transformTranspose);

		XMVECTOR detTransformTranspose;
		XMMATRIX invTransformTranspose = DirectX::XMMatrixInverse(&detTransformTranspose, transformMatrix);
		DirectX::XMStoreFloat4x4(&updateCb.InvTransformMatrix, invTransformTranspose);

		updateCb.Uid[0] = m_instances[i]->uid;

		LightExtension* lightExtension = m_instances[i]->lightExtension.get();
		if (lightExtension)
		{
			const FVector3& emissionIntensity = lightExtension->GetIntensity();
			updateCb.LightEmission[0] = emissionIntensity.X;
			updateCb.LightEmission[1] = emissionIntensity.Y;
			updateCb.LightEmission[2] = emissionIntensity.Z;
		}

		(*m_instanceConstants)[i] = updateCb;
	}

	if (!m_instanceConstants->UpdateToGPU())
	{
		return false;
	}*/

	return true;
}

void StaticMesh::Draw(GraphicsContext* graphicsContext, ID3D12GraphicsCommandList* cmdList, PassType passType, bool useSimpleVertex, bool setTextures)
{
	/*
	if (!graphicsContext || !cmdList)
	{
		return;
	}

	if (m_instances.empty())
	{
		return;
	}
	
	D3D12_GPU_DESCRIPTOR_HANDLE instanceCbHandle;
	if (!m_instanceConstants->BindConstantBufferViewToPipeline(graphicsContext, instanceCbHandle))
	{
		return;
	}

	for (int i = 0; i < m_vbResources.size(); i++)
	{
		const D3D12_VERTEX_BUFFER_VIEW& pVbViews = useSimpleVertex ? m_simpleVertexBufferViews[i] : m_vertexBufferViews[i];
		std::vector<std::unique_ptr<D3DResource>>& vbResourceList = useSimpleVertex ? m_simpleVbResources : m_vbResources;
	
		vbResourceList[i]->CopyToDefaultHeap(cmdList);

		cmdList->SetGraphicsRootDescriptorTable(_passMeshArgumentsMap[passType][MESH_CONSTANT_BUFFER_MESH_ARGUMENT_ID], instanceCbHandle);

		if (setTextures)
		{
			Material* material = m_materials[i].get();
			if (material)
			{
				Texture* baseColorTex = material->GetBaseColorTexture().get();
				Texture* metallicTex = material->GetMetallicTexture().get();
				Texture* roughnessTex = material->GetRoughnessTexture().get();
				Texture* normalTex = material->GetNormalTexture().get();

				if (baseColorTex && baseColorTex->HasCopiedToDefaultHeap())
				{
					D3D12_GPU_DESCRIPTOR_HANDLE baseColorGpuDescHandle;
					if (baseColorTex->BindShaderResourceViewToPipeline(graphicsContext, baseColorGpuDescHandle))
					{
						cmdList->SetGraphicsRootDescriptorTable(_passMeshArgumentsMap[passType][BASE_COLOR_TEX_MESH_ARGUMENT_ID], baseColorGpuDescHandle);
					}
				}
				if (metallicTex && metallicTex->HasCopiedToDefaultHeap())
				{
					D3D12_GPU_DESCRIPTOR_HANDLE metallicGpuDescHandle;
					if (metallicTex->BindShaderResourceViewToPipeline(graphicsContext, metallicGpuDescHandle))
					{
						cmdList->SetGraphicsRootDescriptorTable(_passMeshArgumentsMap[passType][METALLIC_TEX_MESH_ARGUMENT_ID], metallicGpuDescHandle);
					}
				}
				if (roughnessTex && roughnessTex->HasCopiedToDefaultHeap())
				{
					D3D12_GPU_DESCRIPTOR_HANDLE roughnessGpuDescHandle;
					if (roughnessTex->BindShaderResourceViewToPipeline(graphicsContext, roughnessGpuDescHandle))
					{
						cmdList->SetGraphicsRootDescriptorTable(_passMeshArgumentsMap[passType][ROUGHNESS_TEX_MESH_ARGUMENT_ID], roughnessGpuDescHandle);
					}
				}
				if (normalTex && normalTex->HasCopiedToDefaultHeap())
				{
					D3D12_GPU_DESCRIPTOR_HANDLE normalGpuDescHandle;
					if (normalTex->BindShaderResourceViewToPipeline(graphicsContext, normalGpuDescHandle))
					{
						cmdList->SetGraphicsRootDescriptorTable(_passMeshArgumentsMap[passType][NORMAL_TEX_MESH_ARGUMENT_ID], normalGpuDescHandle);
					}
				}
			}
		}
		cmdList->IASetVertexBuffers(0, 1, &pVbViews);
		cmdList->DrawInstanced(m_vertexCounts[i], (UINT)m_instances.size(), 0, 0);
	}*/
}

void StaticMesh::AttachLightExtension(LightExtension* light, const uint32_t& instanceIdx)
{
	/*
	if (light == nullptr)
	{
		return;
	}

	if (instanceIdx >= m_instances.size())
	{
		return;
	}

	m_instances[instanceIdx]->lightExtension = std::unique_ptr<LightExtension>(light);*/
}

bool StaticMesh::StreamInBinary(GraphicsContext* graphicsContext, CommandBuilder* cmdBuilder)
{
	// The object has been streamed in. early out!
	if (m_streamedIn)
	{
		return true;
	}

	/*
	// Build resource for vertex buffers
	for (size_t i = 0; i < m_meshParts.size(); i++)
	{
		MeshPartStruct* meshPartStruct = m_meshParts[i].get();
		auto vbDesc = CD3DX12_RESOURCE_DESC::Buffer(meshPartStruct->bufferSize);

		file->seekg(meshPartStruct->verticesOffset);
		char* meshPartData = (char*)malloc(meshPartStruct->bufferSize);
		file->read(meshPartData, meshPartStruct->vertexCount);

		meshPartStruct->pResource = std::unique_ptr<D3DResource>(new D3DResource(true));
		if (!meshPartStruct->pResource->Initialize(graphicsContext, &vbDesc, meshPartData, meshPartStruct->bufferSize, 0))
		{
			return false;
		}

		// Copy to default heap
		meshPartStruct->pResource->CopyToDefaultHeap(cmdList);

		meshPartStruct->vertexBufferView.BufferLocation = meshPartStruct->pResource->GetDefaultResource()->GetGPUVirtualAddress();
		meshPartStruct->vertexBufferView.StrideInBytes = sizeof(MeshVertexDx);
		meshPartStruct->vertexBufferView.SizeInBytes = meshPartStruct->bufferSize;

		free(meshPartData);
	}
	*/
	m_streamedIn = true;

	return true;
}

bool StaticMesh::CleanUpAfterStreamIn()
{
	for (size_t i = 0; i < m_meshParts.size(); i++)
	{
		// Clean up upload heap resource of mesh parts.
		MeshPartStruct* meshPartStruct = m_meshParts[i].get();
		if (meshPartStruct->pResource)
		{
			meshPartStruct->pResource->CleanUploadResource();
		}
	}

	return true;
}

bool StaticMesh::StreamOut()
{
	return false;
}


void StaticMesh::QueueStreamingTasks(ResourceStreamer* streamer, UINT priority)
{
	if (!streamer)
	{
		return;
	}

	for (auto& material : m_materials)
	{
		if (!material)
		{
			continue;
		}

		std::shared_ptr<Texture> colorTexture = material->GetBaseColorTexture();
		if (colorTexture != nullptr && !colorTexture->HasCopiedToDefaultHeap())
		{
			streamer->StreamIn(colorTexture, priority);
		}

		std::shared_ptr<Texture> metallicTexture = material->GetMetallicTexture();
		if (metallicTexture != nullptr && !metallicTexture->HasCopiedToDefaultHeap())
		{
			streamer->StreamIn(metallicTexture, priority);
		}

		std::shared_ptr<Texture> roughnessTexture = material->GetRoughnessTexture();
		if (roughnessTexture != nullptr && !roughnessTexture->HasCopiedToDefaultHeap())
		{
			streamer->StreamIn(roughnessTexture, priority);
		}

		std::shared_ptr<Texture> normalTexture = material->GetNormalTexture();
		if (normalTexture != nullptr && !normalTexture->HasCopiedToDefaultHeap())
		{
			streamer->StreamIn(normalTexture, priority);
		}
	}
}

