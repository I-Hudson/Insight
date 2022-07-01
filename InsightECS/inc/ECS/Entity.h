#pragma once

#include "Core/TypeAlias.h"
#include "Platform/Platform.h"
#include "ECS/Defines.h"

#include <string>

namespace Insight
{
	namespace ECS
	{
		class IS_ECS EntityManager;

		/// <summary>
		/// Store all relevant data for an entity.
		/// This is not stored on the Entity as the 
		/// </summary>
		struct IS_ECS EntityData
		{
			std::string Name;
			u64 GUID;
		};

		/// <summary>
		/// Entity is a simple handle for an entity. This class
		/// shouldn't hold/store any critical data.
		/// </summary>
		class IS_ECS Entity
		{
		public:

			void SetName(std::string name);


			std::string GetName() const;
			int GetId() const;
			u64 GetGuid() const;

			bool IsVaild() const;
			template<typename Component>
			bool HasComponent() const
			{

				return false;
			}

		private:
			int m_id = -1;
			EntityManager* m_entityManager = nullptr;
			friend class EntityManager;
		};

		const Entity c_InvalidEntity = Entity();
	}
}