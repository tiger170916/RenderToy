#pragma once

#include "../Includes.h"
#include "../ConstantBuffer.h"
#include "../LightStructs.h"
#include "LightExtension.h"

class SpotLight : public LightExtension
{
friend class LightFactory;

private:
	FRotator m_rotator;

	float m_aspectRatio;

	float m_fov;

	XMMATRIX m_projectionMatrix;

	std::unique_ptr<D3DResource> m_vertexBufferResource;

	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView = {};

	std::unique_ptr<ConstantBuffer<LightViewFrustumConstantsDX>> m_lightViewFrustumConstantBuffer;

protected:
	SpotLight(float effectiveRange, FVector3 position, FVector3 intensity, uint32_t uid, FRotator rotator, float aspectRatio, float fov);

public:
	~SpotLight();

	const XMMATRIX GetProjectionMatrix() const { return m_projectionMatrix; }

	const FRotator GetRotator() const { return m_rotator; }

	virtual bool Initialize(GraphicsContext* graphicsContext) override;

	void DrawEffectiveFrustum(GraphicsContext* graphicsContext, ID3D12GraphicsCommandList* cmdList, FVector3 parentTransform);
};