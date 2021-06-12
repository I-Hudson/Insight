#include "ispch.h"
#include "Engine/Component/SkinnedMeshComponent.h"
#include "Engine/Component/AnimatorComponent.h"
#include "Engine/Entity/Entity.h"

SkinnedMeshComponent::SkinnedMeshComponent()
{}

SkinnedMeshComponent::SkinnedMeshComponent(ComponentManager* componentManager, ComponentID componentID, ComponentType componentType, EntityManager* entityManager, EntityID entity)
	: MeshComponent(componentManager, componentID, componentType, entityManager, entity)
{ }

SkinnedMeshComponent::~SkinnedMeshComponent()
{}

void SkinnedMeshComponent::OnUpdate(const float& a_deltaTime)
{
	if (!GetEntity().HasComponent<AnimatorComponent>())
	{
		return;
	}

	SkinnedMeshComponentData& data = GetComponentData<SkinnedMeshComponentData>();
	if (data.MatrixStorageBuffer == nullptr)
	{
		// Init the storage bufer
		data.MatrixStorageBuffer = Insight::Graphics::GPUBuffer::New();
	}

	std::vector<glm::mat4> bones = GetEntity().GetComponent<AnimatorComponent>().GetFinalBoneMatrices();
	const Insight::Graphics::GPUBufferDesc& desc = data.MatrixStorageBuffer->GetDesc();
	if (desc.Flags == Insight::Graphics::GPUBufferFlags::NONE || desc.Size / desc.Stride != bones.size())
	{
		// New animator or different amount of bones on this entity.
		data.MatrixStorageBuffer->Init(Insight::Graphics::GPUBufferDesc::StorageBuffer(sizeof(glm::mat4), bones.size(), bones.data()));
	}
	else
	{
		// Update the buffer with the new information. Maybe check if the info is the same or even needs updating.
		data.MatrixStorageBuffer->SetData(bones.data(), desc.Size);
	}
}

void SkinnedMeshComponent::OnDestroy()
{
	SkinnedMeshComponentData& data = GetComponentData<SkinnedMeshComponentData>();
	if (data.MatrixStorageBuffer)
	{
		data.MatrixStorageBuffer->ReleaseGPU();
		::Delete(data.MatrixStorageBuffer);
	}
}