#pragma once

namespace Insight
{
	namespace ECS
	{
		class Entity;

		/// @brief Interface for creating an entity hierarchy.
		class ICreateEntityHierarchy
		{
		public:
			/// @brief Return the top parent entity and create a hierarchy.
			/// @return Entity*
			virtual Entity* CreateEntityHierarchy() = 0;
		};
	}
}