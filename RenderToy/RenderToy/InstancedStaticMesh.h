#pragma once

#include "Includes.h"
#include "IMesh.h"
#include "Lights/LightExtension.h"

class InstancedStaticMesh : public IMesh
{
// Note that instanced mesh is not controllable
private:

struct InstanceStruct
{
	uint32_t uid;
	Transform transform;
	std::unique_ptr<LightExtension> lightExtension = nullptr;
};

private:
	std::vector<std::unique_ptr<InstanceStruct>> m_instances;

public: 
	InstancedStaticMesh(std::string meshName);

	~InstancedStaticMesh();

	/// <summary>
	/// Stream out from gpu
	/// </summary>
	bool StreamOut();

	void AddInstance(Transform transform);

	bool AttachLightExtension(uint32_t instanceIdx, std::unique_ptr<LightExtension> lightExt);

	inline uint32_t GetNumInstances() const { return (uint32_t)m_instances.size(); }

	bool GetInstanceTransform(const uint32_t& idx, Transform& transform) const;

	bool GetInstanceUid(const uint32_t& idx, uint32_t& outUid) const;

	LightExtension* GetLightExtension(const uint32_t& idx);

	// IDrawable interface implementation
	virtual void Draw(GraphicsContext* graphicsContext, MaterialManager* materialManager, TextureManager2* textureManager, ID3D12GraphicsCommandList* cmdList, PassType passType) override;

	// IMesh interface implementation
	virtual bool UpdateBuffersForFrame() override;
};