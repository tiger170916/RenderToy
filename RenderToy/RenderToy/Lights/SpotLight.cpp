#include "SpotLight.h"
#include "../MeshStructs.h"

SpotLight::SpotLight(float effectiveRange, FVector3 position, FVector3 intensity, uint32_t uid, FRotator rotator, float aspectRatio, float fov)
	:LightExtension(effectiveRange, position, intensity, uid), m_rotator(rotator), m_aspectRatio(aspectRatio), m_fov(fov)
{
	m_lightType = LightType::LightType_Spot;

	m_projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(fov * DirectX::XM_PI, aspectRatio, m_nearPlane, effectiveRange);
}

bool SpotLight::Initialize(GraphicsContext* graphicsContext)
{
	if (!graphicsContext)
	{
		return false;
	}

	float nearPlaneHalfHeight = m_nearPlane * tan(m_fov * DirectX::XM_PI / 2.0f);
	float nearPlaneHalfWidth = nearPlaneHalfHeight * m_aspectRatio;
	float farPlaneHalfHeight = m_effectiveRange * tan(m_fov * DirectX::XM_PI / 2.0f);
	float farPlaneHalfWidth = farPlaneHalfHeight * m_aspectRatio;


	// Build view frustum.
	FVector3 n1 = FVector3(-nearPlaneHalfWidth, nearPlaneHalfHeight, m_nearPlane);
	FVector3 n2 = FVector3(nearPlaneHalfWidth, nearPlaneHalfHeight, m_nearPlane);
	FVector3 n3 = FVector3(nearPlaneHalfWidth, -nearPlaneHalfHeight, m_nearPlane);
	FVector3 n4 = FVector3(-nearPlaneHalfWidth, -nearPlaneHalfHeight, m_nearPlane);

	FVector3 f1 = FVector3(-farPlaneHalfWidth, farPlaneHalfHeight, m_effectiveRange);
	FVector3 f2 = FVector3(farPlaneHalfWidth, farPlaneHalfHeight, m_effectiveRange);
	FVector3 f3 = FVector3(farPlaneHalfWidth, -farPlaneHalfHeight, m_effectiveRange);
	FVector3 f4 = FVector3(-farPlaneHalfWidth, -farPlaneHalfHeight, m_effectiveRange);

	std::vector<FVector3> vertices = { 
		n4, n1, n2,
		n4, n2, n3,
		f2, f4, f1,
		f2, f3, f4,
		n2, f2, n3,
		n3, f2, n3,
		f4, f1, n1,
		f4, n1, n4 };

	std::vector<LightViewFrustumVertexDx> verticesDx;
	for (size_t i = 0; i < vertices.size(); i++)
	{
		verticesDx.push_back(LightViewFrustumVertexDx{
			{vertices[i].X, vertices[i].Y, vertices[i].Z}});
	}

	m_vertexBufferResource = std::unique_ptr<D3DResource>(new D3DResource(true));

	// Create vb 
	auto vbDesc = CD3DX12_RESOURCE_DESC::Buffer(verticesDx.size() * sizeof(LightViewFrustumVertexDx));

	if (!m_vertexBufferResource->Initialize(graphicsContext, &vbDesc, verticesDx.data(), (UINT)verticesDx.size() * sizeof(LightViewFrustumVertexDx), 0))
	{
		return false;
	}

	// Create vb view
	m_vertexBufferView.BufferLocation = m_vertexBufferResource->GetDefaultResource()->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = sizeof(LightViewFrustumVertexDx);
	m_vertexBufferView.SizeInBytes = (UINT)verticesDx.size() * sizeof(LightViewFrustumVertexDx);

	return true;
}

void SpotLight::DrawEffectiveFrustum(ID3D12GraphicsCommandList* cmdList)
{
	if (!cmdList)
	{
		return;
	}

	m_vertexBufferResource->CopyToDefaultHeap(cmdList);
	cmdList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	cmdList->DrawInstanced(24, 1, 0, 0);
}

SpotLight::~SpotLight() {}