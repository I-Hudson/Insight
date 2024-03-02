#include "ECS/ComponentArray.h"
#include "Core/Memory.h"
#include "Core/Profiler.h"

namespace Insight
{
	namespace ECS
	{
#ifdef IS_ECS_ENABLED
		ComponentArrayManager::ComponentArrayManager(ECSWorld* const ecsWorld)
			: m_ecsWorld(ecsWorld)
		{ }

		void ComponentArrayManager::Update(float deltaTime)
		{
			IS_PROFILE_FUNCTION();
			for (const auto& pair : m_componentArrays)
			{
				IS_PROFILE_SCOPE("Single component array");
				pair.second->Update(deltaTime);
			}
		}

		void ComponentArrayManager::RemoveComponent(Entity entity, ComponentHandle& handle)
		{
			IComponentArray* componentArray = GetComponentArray(handle.GetType());
			if (componentArray == nullptr)
			{
				return;
			}
			componentArray->RemoveComponent(entity, handle);
		}
#endif
		//---------------------------------------
		// IComponentArray
		//---------------------------------------
		IComponentArray::IComponentArray(Reflect::TypeInfo typeInfo)
			: m_typeInfo(std::move(typeInfo))
		{ }
		IComponentArray::~IComponentArray()
		{
		}

		const Reflect::TypeInfo& IComponentArray::GetTypeInfo() const
		{
			return m_typeInfo;
		}

		//---------------------------------------
		// ComponentManager
		//---------------------------------------
		ComponentManager::ComponentManager()
		{
		}

		ComponentManager::~ComponentManager()
		{
			for (size_t i = 0; i < m_componentArrays.size(); ++i)
			{
				::Delete(m_componentArrays.at(i));
			}
		}

		void ComponentManager::OnEarlyUpdate() const
		{
			for (size_t i = 0; i < m_componentArrays.size(); ++i)
			{
				IComponentArray* componentArray = m_componentArrays.at(i);
				componentArray->OnEarlyUpdate();
			}
		}
		void ComponentManager::OnUpdate(const float deltaTime) const
		{
			for (size_t i = 0; i < m_componentArrays.size(); ++i)
			{
				IComponentArray* componentArray = m_componentArrays.at(i);
				componentArray->OnUpdate(deltaTime);
			}
		}
		void ComponentManager::OnLateUpdate() const
		{
			for (size_t i = 0; i < m_componentArrays.size(); ++i)
			{
				IComponentArray* componentArray = m_componentArrays.at(i);
				componentArray->OnLateUpdate();
			}
		}
	}
}