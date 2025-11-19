#include "StaticMeshComponent.h"
#include "ExtensionFactory.h"
#include "StaticMesh2.h"
#include "InstancedStaticMesh.h"

StaticMeshComponent::StaticMeshComponent(std::string name, Object* pParent)
	: SceneObjectComponent(name, pParent)
{}

bool StaticMeshComponent::ConstructComponentFromResource(ResourceCompilerModule::StaticMeshComponent* rcStaticMeshComponent)
{
	if (rcStaticMeshComponent == nullptr)
	{
		return false;
	}

	std::vector<ResourceCompilerModule::StaticMeshInstance*> instances;
	rcStaticMeshComponent->GetInstances(instances);
	if (rcStaticMeshComponent->GetInstanceCount() == 0)
	{
		return false;
	}

	m_name = rcStaticMeshComponent->GetName();

	if (rcStaticMeshComponent->GetInstanceCount() == 1) 
	{
		std::unique_ptr<StaticMesh2> staticMesh = std::make_unique<StaticMesh2>(rcStaticMeshComponent->GetName(), this);
		ResourceCompilerModule::StaticMeshInstance*& instance = instances[0];
		Transform transform = {};
		instance->GetPosition(transform.Translation.X, transform.Translation.Y, transform.Translation.Z);
		instance->GetRotation(transform.Rotation.Pitch, transform.Rotation.Yaw, transform.Rotation.Roll);
		instance->GetScale(transform.Scale.X, transform.Scale.Y, transform.Scale.Z);
		
		SetTransform(transform);

		// Get light ext
		if (instance->HasLightExtension())
		{
			ResourceCompilerModule::LightExtension* rcLightExt = instance->GetLightExtension();
			float offset[3] = {};
			float rotation[3] = {};
			float color[3] = {};
			rcLightExt->GetOffset(offset[0], offset[1], offset[2]);
			rcLightExt->GetRotation(rotation[0], rotation[1], rotation[2]);
			rcLightExt->GetColor(color[0], color[1], color[2]);
			float intensity = rcLightExt->GetIntensity();
			float attenuationRadius = rcLightExt->GetAttenuationRadius();
			float aspectRatio = rcLightExt->GetAspectRatio();
			float fov = rcLightExt->GetFov();
			
			std::unique_ptr<LightExtension> lightExtension = ExtensionFactory::SpawnLightExtension(
				offset,rotation,color, intensity, attenuationRadius, aspectRatio, fov);

			if (lightExtension)
			{
				staticMesh->AttachLightExtension(std::move(lightExtension));
			}
		}

		m_mesh = std::move(staticMesh);
	}
	else
	{
		std::unique_ptr<InstancedStaticMesh> instancedStaticMesh = std::make_unique<InstancedStaticMesh>(rcStaticMeshComponent->GetName());
		for (uint32_t instanceIdx = 0; instanceIdx < (uint32_t)instances.size(); instanceIdx++)
		{
			ResourceCompilerModule::StaticMeshInstance*& instance = instances[instanceIdx];
			Transform transform = {};
			instance->GetPosition(transform.Translation.X, transform.Translation.Y, transform.Translation.Z);
			instance->GetRotation(transform.Rotation.Pitch, transform.Rotation.Yaw, transform.Rotation.Roll);
			instance->GetScale(transform.Scale.X, transform.Scale.Y, transform.Scale.Z);
			instancedStaticMesh->AddInstance(transform);

			if (instance->HasLightExtension())
			{
				ResourceCompilerModule::LightExtension* rcLightExt = instance->GetLightExtension();
				float offset[3] = {};
				float rotation[3] = {};
				float color[3] = {};
				rcLightExt->GetOffset(offset[0], offset[1], offset[2]);
				rcLightExt->GetRotation(rotation[0], rotation[1], rotation[2]);
				rcLightExt->GetColor(color[0], color[1], color[2]);
				float intensity = rcLightExt->GetIntensity();
				float attenuationRadius = rcLightExt->GetAttenuationRadius();
				float aspectRatio = rcLightExt->GetAspectRatio();
				float fov = rcLightExt->GetFov();

				std::unique_ptr<LightExtension> lightExtension = ExtensionFactory::SpawnLightExtension(
					offset, rotation, color, intensity, attenuationRadius, aspectRatio, fov);

				if (lightExtension)
				{
					instancedStaticMesh->AttachLightExtension(instanceIdx, std::move(lightExtension));
				}
			}
		}

		m_mesh = std::move(instancedStaticMesh);
	}

	return true;
}