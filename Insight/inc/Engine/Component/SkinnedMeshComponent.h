#pragma once

#include "Engine/Component/MeshComponent.h"
#include "Engine/Graphics/GPUBuffer.h"

struct SkinnedMeshComponentData : public ComponentData
{
	Insight::Graphics::GPUBuffer* MatrixStorageBuffer = nullptr;
};

class SkinnedMeshComponent : public MeshComponent
{
public:
	SkinnedMeshComponent();
	SkinnedMeshComponent(ComponentManager* componentManager, ComponentID componentID, ComponentType componentType, EntityManager* entityManager, EntityID entity);
	virtual ~SkinnedMeshComponent() override;

	virtual void OnUpdate(const float& a_deltaTime) override;
	virtual void OnDraw(Insight::Graphics::RenderListView* renderList, const glm::mat4& worldTransform, const Insight::Maths::Frustum& cameraFrustum) override;

	virtual void OnBeginPlay() override;
	virtual void OnEndPlay() override;

	virtual void OnDestroy() override;
};