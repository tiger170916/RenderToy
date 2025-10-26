#include "InstancedStaticMesh.h"
#include "UidGenerator.h"
#include "Macros.h"
#include "GraphicsUtils.h"

InstancedStaticMesh::InstancedStaticMesh(std::string meshName)
	: IMesh(meshName)
{

}

InstancedStaticMesh::~InstancedStaticMesh() {}

bool InstancedStaticMesh::StreamOut() 
{
	if (!m_isUploadedToGpu)
	{
		return true;
	}
	return false;
}

void InstancedStaticMesh::AddInstance(Transform transform)
{
	std::unique_ptr<InstanceStruct> instance = std::make_unique<InstanceStruct>();
	instance->transform = transform;
	instance->uid = UidGenerator::Get()->GenerateUid();
	m_instances.push_back(std::move(instance));
}

bool InstancedStaticMesh::AttachLightExtension(uint32_t instanceIdx, std::unique_ptr<LightExtension> lightExt)
{
	if (instanceIdx >= m_instances.size())
	{
		return false;
	}

	if (m_instances[instanceIdx]->lightExtension != nullptr)
	{
		return false;
	}

	m_instances[instanceIdx]->lightExtension = std::move(lightExt);
	return true;
}

bool InstancedStaticMesh::GetInstanceTransform(const uint32_t& idx, Transform& transform) const
{
	if (idx >= m_instances.size() || !m_instances[idx])
	{
		return false;
	}


	transform = m_instances[idx]->transform;
	return true;
}

bool InstancedStaticMesh::GetInstanceUid(const uint32_t& idx, uint32_t& outUid) const
{
	if (idx >= m_instances.size() || !m_instances[idx])
	{
		return false;
	}

	outUid = m_instances[idx]->uid;
	return true;
}

LightExtension* InstancedStaticMesh::GetLightExtension(const uint32_t& idx)
{
	if (idx >= m_instances.size())
	{
		return nullptr;
	}

	if (m_instances[idx]->lightExtension)
	{
		return m_instances[idx]->lightExtension.get();
	}

	return nullptr;
}

// IDrawable interface implementation
void InstancedStaticMesh::Draw(GraphicsContext* graphicsContext, MaterialManager* materialManager, TextureManager2* textureManager, ID3D12GraphicsCommandList* cmdList, PassType passType)
{
	if (!graphicsContext || !cmdList)
	{
		return;
	}

	if (!m_isUploadedToGpu)
	{
		return;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE instanceCbHandle;
	if (!m_instanceConstants->BindConstantBufferViewToPipeline(graphicsContext, instanceCbHandle))
	{
		return;
	}

	bool setTextures = passType == PassType::GEOMETRY_PASS;

	DescriptorHeapManager* descHeapManager = graphicsContext->GetDescriptorHeapManager();
	for (auto& meshPart : m_meshParts)
	{
		cmdList->SetGraphicsRootDescriptorTable(_passMeshArgumentsMap[passType][MESH_CONSTANT_BUFFER_MESH_ARGUMENT_ID], instanceCbHandle);

		if (setTextures)
		{
			Material2* material = materialManager->GetMaterial(meshPart->materialName);
			if (material)
			{
				Texture2* baseColorTex = textureManager->GetTexture(material->GetBaseColorTexName());
				if (baseColorTex && baseColorTex->IsStreamedIn())
				{
					D3D12_GPU_DESCRIPTOR_HANDLE baseColorGpuDescHandle;
					if (descHeapManager->BindCbvSrvUavToPipeline(baseColorTex->GetSrvId(), baseColorGpuDescHandle))
					{
						cmdList->SetGraphicsRootDescriptorTable(_passMeshArgumentsMap[passType][BASE_COLOR_TEX_MESH_ARGUMENT_ID], baseColorGpuDescHandle);
					}
				}

				Texture2* metallicTex = textureManager->GetTexture(material->GetMetallicTexName());
				if (metallicTex && metallicTex->IsStreamedIn())
				{
					D3D12_GPU_DESCRIPTOR_HANDLE metallicTexGpuDescHandle;
					if (descHeapManager->BindCbvSrvUavToPipeline(metallicTex->GetSrvId(), metallicTexGpuDescHandle))
					{
						cmdList->SetGraphicsRootDescriptorTable(_passMeshArgumentsMap[passType][METALLIC_TEX_MESH_ARGUMENT_ID], metallicTexGpuDescHandle);
					}
				}

				Texture2* roughnessTex = textureManager->GetTexture(material->GetRoughnessTexName());
				if (roughnessTex && roughnessTex->IsStreamedIn())
				{
					D3D12_GPU_DESCRIPTOR_HANDLE roughnessTexGpuDescHandle;
					if (descHeapManager->BindCbvSrvUavToPipeline(roughnessTex->GetSrvId(), roughnessTexGpuDescHandle))
					{
						cmdList->SetGraphicsRootDescriptorTable(_passMeshArgumentsMap[passType][ROUGHNESS_TEX_MESH_ARGUMENT_ID], roughnessTexGpuDescHandle);
					}
				}

				Texture2* normalTex = textureManager->GetTexture(material->GetNormalTexName());
				if (normalTex && normalTex->IsStreamedIn())
				{
					D3D12_GPU_DESCRIPTOR_HANDLE normalTexGpuDescHandle;
					if (descHeapManager->BindCbvSrvUavToPipeline(normalTex->GetSrvId(), normalTexGpuDescHandle))
					{
						cmdList->SetGraphicsRootDescriptorTable(_passMeshArgumentsMap[passType][NORMAL_TEX_MESH_ARGUMENT_ID], normalTexGpuDescHandle);
					}
				}
			}
		}



		cmdList->IASetVertexBuffers(0, 1, &meshPart->vertexBufferView);
		cmdList->DrawInstanced(meshPart->vertexCount, 1, 0, 0);
	}
}

// IMesh interface implementation
bool InstancedStaticMesh::UpdateBuffersForFrame()
{
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
	}

	return true;
}