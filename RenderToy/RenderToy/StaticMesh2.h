#pragma once

#include "Includes.h"
#include "IMesh.h"
#include "IControllable.h"
#include "GraphicsContext.h"
#include "D3DResource.h"
#include "CommandBuilder.h"
#include "ConstantBuffer.h"
#include "MeshStructs.h"
#include "Vectors.h"
#include "lights/LightExtension.h"
#include <fstream>

class StaticMesh2 : public IMesh, public IControllable
{
private:
	std::unique_ptr<LightExtension> m_lightExtension = nullptr;

public:
	StaticMesh2(std::string meshName);

	~StaticMesh2();

	/// <summary>
	/// Stream out from gpu
	/// </summary>
	bool StreamOut();

	//bool AttachLightExtension(uint32_t instanceIdx, std::unique_ptr<LightExtension>& lightExt);
	void AttachLightExtension(std::unique_ptr<LightExtension> lightExt);

	inline LightExtension* GetLightExtension() { return m_lightExtension.get(); }

	// IControllable interface implementation
	virtual void ProcessInput(DirectX::Mouse::State, DirectX::Keyboard::State, float deltaTime) override;

	// IDrawable interface implementation
	virtual void Draw(GraphicsContext* graphicsContext, MaterialManager* materialManager, TextureManager2* textureManager, ID3D12GraphicsCommandList* cmdList, PassType passType) override;

	// IMesh interface implementation
	virtual bool UpdateBuffersForFrame() override;
};