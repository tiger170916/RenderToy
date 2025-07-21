#include "Material.h"

Material::Material()
{
	
}

bool Material::Initialize(GraphicsContext* graphicsContext)
{
	m_materialConstantBuffer = std::unique_ptr<ConstantBuffer<MaterialConstants>>(new ConstantBuffer<MaterialConstants>());
	if (!m_materialConstantBuffer->Initialize(graphicsContext))
	{
		return false;
	}

	m_initialized = true;

	return true;
}

Material::~Material()
{

}