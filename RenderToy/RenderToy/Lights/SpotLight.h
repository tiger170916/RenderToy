#pragma once

#include "../Includes.h"
#include "LightExtension.h"

class SpotLight : public LightExtension
{
friend class LightFactory;

private:
	FRotator m_rotator;

	float m_aspectRatio;

	float m_fov;

	float m_nearPlane = 0.1f;

	XMMATRIX m_projectionMatrix;

	std::unique_ptr<D3DResource> m_vertexBufferResource;

	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView = {};

protected:
	SpotLight(float effectiveRange, FVector3 position, FVector3 intensity, uint32_t uid, FRotator rotator, float aspectRatio, float fov);

public:
	~SpotLight();

	const XMMATRIX GetProjectionMatrix() const { return m_projectionMatrix; }

	const FRotator GetRotator() const { return m_rotator; }

	virtual bool Initialize(GraphicsContext* graphicsContext) override;

	void DrawEffectiveFrustum(ID3D12GraphicsCommandList* cmdList);
};