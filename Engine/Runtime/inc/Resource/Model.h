#pragma once

#include "Resource/Resource.h"
#include "ECS/ICreateEntityHierarchy.h"

#include "Graphics/RHI/RHI_Buffer.h"

namespace Insight
{
	namespace Runtime
	{
		class Mesh;
		class Material;
		class AssimpLoader;

		class IS_RUNTIME Model : public IResource, public ECS::ICreateEntityHierarchy
		{
			REGISTER_RESOURCE(Model);
		public:

			Mesh* GetMesh() const;
			Mesh* GetMeshByIndex(u32 index) const;

			//--ECS::ICreateEntityHierarchy
			virtual ECS::Entity* CreateEntityHierarchy() override;
			//--ECS::ICreateEntityHierarchy

		private:
			//--IResource 
			/// @brief Handle loading the resource from disk.
			/// @param file_path 
			virtual void Load() override;
			/// @brief Handle unloading the resource from memory.
			virtual void UnLoad() override;
			/// @brief Handle saving a Model to disk.
			/// @param file_path 
			virtual void Save(const std::string& file_path) override;
			//--IResource 

		private:
			std::vector<Mesh*> m_meshes;
			std::vector<Material*> m_materials;
			Graphics::RHI_Buffer* m_vertex_buffer = nullptr;
			Graphics::RHI_Buffer* m_index_buffer = nullptr;
			// std::vector<Materials> m_materials

			friend class AssimpLoader;
		};
	}
}