#include "StaticMesh.h"

StaticMesh::StaticMesh()
{

}

StaticMesh::~StaticMesh()
{

}

void StaticMesh::AddMaterial(Material* material)
{
	m_materials.push_back(std::unique_ptr<Material>(material));
}

void StaticMesh::AddTriangle(const int& part, const MeshVertex& v1, const MeshVertex& v2, const MeshVertex& v3)
{
	if (!m_meshParts.contains(part))
	{
		m_meshParts[part] = std::vector<MeshVertex>();
	}

	m_meshParts[part].push_back(v1);
	m_meshParts[part].push_back(v2);
	m_meshParts[part].push_back(v3);
}

void StaticMesh::AddInstance(const Transform& transform)
{
	m_instances.push_back(transform);
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
		for (int i = 0; i < vertices.size(); i++)
		{
			const MeshVertex& vert = vertices[i];
			MeshVertexDx vertDx = {};
			vertDx.position[0] = vert.Position.X;
			vertDx.position[1] = vert.Position.Y;
			vertDx.position[2] = vert.Position.Z;
			vertDx.UV0[0] = vert.UvSets[0].X;
			vertDx.UV0[1] = vert.UvSets[0].Y;
			verticesDx.push_back(vertDx);
		}

		D3DResource* vertexBufferResource = new D3DResource(true);
		// Create vb 
		auto vbDesc = CD3DX12_RESOURCE_DESC::Buffer(verticesDx.size() * sizeof(MeshVertexDx));
		if (!vertexBufferResource->Initialize(graphicsContext, &vbDesc, verticesDx.data(), (UINT)verticesDx.size() * sizeof(MeshVertexDx), 0))
		{
			return false;
		}
		m_vbResources.push_back(std::unique_ptr<D3DResource>(vertexBufferResource));

		// Create vb view
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
		vertexBufferView.BufferLocation = vertexBufferResource->GetDefaultResource()->GetGPUVirtualAddress();
		vertexBufferView.StrideInBytes = sizeof(MeshVertexDx);
		vertexBufferView.SizeInBytes = (UINT)verticesDx.size() * sizeof(MeshVertexDx);

		m_vertexBufferViews.push_back(vertexBufferView);
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

	return true;
}

void StaticMesh::Draw(GraphicsContext* graphicsContext, ID3D12GraphicsCommandList* cmdList, bool setTextures)
{
	if (!graphicsContext || !cmdList)
	{
		return;
	}


	for (int i = 0; i < m_instances.size(); i++)
	{
		MeshInstanceConstants updateCb = {};
		
		Transform& transform = m_instances[i];
		XMMATRIX transformMatrix = XMMatrixIdentity();
		XMMATRIX rotation = XMMatrixRotationRollPitchYaw(transform.Rotation.Pitch, transform.Rotation.Yaw, transform.Rotation.Roll);
		XMMATRIX translation = XMMatrixTranslation(transform.Translation.X, transform.Translation.Y, transform.Translation.Z);
		XMMATRIX scale = XMMatrixScaling(transform.Scale.X, transform.Scale.Y, transform.Scale.Z);
		transformMatrix = scale * rotation * translation;
		
		DirectX::XMStoreFloat4x4(&updateCb.TransformMatrix, DirectX::XMMatrixTranspose(transformMatrix));
		(*m_instanceConstants)[i] = updateCb;
	}

	if (!m_instanceConstants->UpdateToGPU())
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
		const D3D12_VERTEX_BUFFER_VIEW* pVbViews = &m_vertexBufferViews[i];
		m_vbResources[i]->CopyToDefaultHeap(cmdList);
		cmdList->SetGraphicsRootDescriptorTable(1, instanceCbHandle);

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
						cmdList->SetGraphicsRootDescriptorTable(3, baseColorGpuDescHandle);
					}
				}
				if (metallicTex && metallicTex->HasCopiedToDefaultHeap())
				{
					D3D12_GPU_DESCRIPTOR_HANDLE metallicGpuDescHandle;
					if (metallicTex->BindShaderResourceViewToPipeline(graphicsContext, metallicGpuDescHandle))
					{
						cmdList->SetGraphicsRootDescriptorTable(4, metallicGpuDescHandle);
					}
				}
				if (roughnessTex && roughnessTex->HasCopiedToDefaultHeap())
				{
					D3D12_GPU_DESCRIPTOR_HANDLE roughnessGpuDescHandle;
					if (roughnessTex->BindShaderResourceViewToPipeline(graphicsContext, roughnessGpuDescHandle))
					{
						cmdList->SetGraphicsRootDescriptorTable(5, roughnessGpuDescHandle);
					}
				}
				if (normalTex && normalTex->HasCopiedToDefaultHeap())
				{
					D3D12_GPU_DESCRIPTOR_HANDLE normalGpuDescHandle;
					if (normalTex->BindShaderResourceViewToPipeline(graphicsContext, normalGpuDescHandle))
					{
						cmdList->SetGraphicsRootDescriptorTable(6, normalGpuDescHandle);
					}
				}
			}
		}
		cmdList->IASetVertexBuffers(0, 1, pVbViews);
		cmdList->DrawInstanced(m_vertexCounts[i], (UINT)m_instances.size(), 0, 0);
	}
}

void StaticMesh::AttachLightExtension(LightExtension* light)
{
	if (light == nullptr)
	{
		return;
	}

	m_lightExtensions.push_back(std::shared_ptr<LightExtension>(light));
}

bool StaticMesh::StreamIn(GraphicsContext* graphicsContext)
{
	/*
	for (auto& material : m_materials)
	{
		if (!material)
		{
			continue;
		}

		std::shared_ptr<Texture> colorTexture = material->GetBaseColorTexture();
		if (colorTexture != nullptr && !colorTexture->HasCopiedToDefaultHeap())
		{
			colorTexture->StreamIn(graphicsContext);
		}
	}*/

	return true;
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

bool StaticMesh::StreamOut()
{
	return false;
}

bool StaticMesh::ScheduleForCopyToDefaultHeap(ID3D12GraphicsCommandList* cmdList)
{
	return true;
}