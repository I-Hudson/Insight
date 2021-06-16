#include "ispch.h"
#include "Engine/Component/SkinnedMeshComponent.h"
#include "Engine/Component/AnimatorComponent.h"
#include "Engine/Entity/Entity.h"
#include "Engine/Graphics/RenderList.h"

SkinnedMeshComponent::SkinnedMeshComponent()
{}

SkinnedMeshComponent::SkinnedMeshComponent(ComponentManager* componentManager, ComponentID componentID, ComponentType componentType, EntityManager* entityManager, EntityID entity)
	: MeshComponent(componentManager, componentID, componentType, entityManager, entity)
{ }

SkinnedMeshComponent::~SkinnedMeshComponent()
{}

void SkinnedMeshComponent::OnUpdate(const float& a_deltaTime)
{
	bool hasAnimator = GetEntity().HasComponent<AnimatorComponent>();
	if (!hasAnimator)
	{
		return;
	}
	AnimatorComponent& animator = GetEntity().GetComponent<AnimatorComponent>();
	if (!animator.GetSkelton() || !animator.GetCurrentAnimation())
	{
		return;
	}

	SkinnedMeshComponentData& data = GetComponentData<SkinnedMeshComponentData>();
	std::vector<glm::mat4> bones = GetEntity().GetComponent<AnimatorComponent>().GetFinalBoneMatrices();
	const Insight::Graphics::GPUBufferDesc& desc = data.MatrixStorageBuffer->GetDesc();
	if (desc.Flags == Insight::Graphics::GPUBufferFlags::NONE || desc.Size / desc.Stride != bones.size())
	{
		// New animator or different amount of bones on this entity.
		data.MatrixStorageBuffer->Init(Insight::Graphics::GPUBufferDesc::StorageBuffer(sizeof(glm::mat4), bones.size(), bones.data()));
		data.MatrixStorageBuffer->SetName("SkinnedMeshMatrixBuffer");
	}
	else
	{
		// Update the buffer with the new information. Maybe check if the info is the same or even needs updating.
		data.MatrixStorageBuffer->SetData(bones.data(), desc.Size);
	}
}

void SkinnedMeshComponent::OnDraw(Insight::Graphics::RenderListView* renderList, const glm::mat4& worldTransform, const Insight::Maths::Frustum& cameraFrustum)
{
	u32 meshCount = renderList->DrawCalls.size();
	MeshComponent::OnDraw(renderList, worldTransform, cameraFrustum);

	SkinnedMeshComponentData& data = GetComponentData<SkinnedMeshComponentData>();
	for (u32 i = meshCount; i < renderList->DrawCalls.size(); ++i)
	{
		if (!data.MatrixStorageBuffer)
		{
			renderList->DrawCalls.at(i).Skinned.BoneMatrices = nullptr;
		}
		else
		{
			renderList->DrawCalls.at(i).Skinned.BoneMatrices = 
				data.MatrixStorageBuffer->GetDesc().Flags != Insight::Graphics::GPUBufferFlags::NONE ?
				data.MatrixStorageBuffer : nullptr;
		}
	}
}

void SkinnedMeshComponent::OnBeginPlay()
{
	SkinnedMeshComponentData& data = GetComponentData<SkinnedMeshComponentData>();
	if (!data.MatrixStorageBuffer)
	{
		// Init the storage bufer
		data.MatrixStorageBuffer = Insight::Graphics::GPUBuffer::New();
	}
}

void SkinnedMeshComponent::OnEndPlay()
{
	SkinnedMeshComponentData& data = GetComponentData<SkinnedMeshComponentData>();
	if (data.MatrixStorageBuffer)
	{
		data.MatrixStorageBuffer->ReleaseGPU();
		::Delete(data.MatrixStorageBuffer);
		data.MatrixStorageBuffer = nullptr;
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