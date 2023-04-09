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
			Model(std::string_view filePath);
			virtual ~Model() override;

			IS_SERIALISABLE_H(Model)

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

	namespace Serialisation
	{
		struct ModelMesh {};
		template<>
		struct ComplexSerialiser<ModelMesh, std::vector<Runtime::Mesh*>, Runtime::Model>
		{
			void operator()(ISerialiser* serialiser, std::vector<Runtime::Mesh*>& meshes, Runtime::Model* model) const;
		};

		struct ModelMaterial {};
		template<>
		struct ComplexSerialiser<ModelMaterial, std::vector<Runtime::Material*>, Runtime::Model>
		{
			void operator()(ISerialiser* serialiser, std::vector<Runtime::Material*>& materials, Runtime::Model* model) const;
		};
	}

	OBJECT_SERIALISER(Runtime::Model, 5,
		SERIALISE_BASE(Runtime::IResource, 1, 0)
		SERIALISE_VECTOR_OBJECT(Runtime::Mesh, m_meshes, 2, 3)
		SERIALISE_VECTOR_OBJECT(Runtime::Material, m_materials, 4, 5)
		SERIALISE_COMPLEX(Serialisation::ModelMaterial, m_materials, 5, 0)
		SERIALISE_COMPLEX(Serialisation::ModelMesh, m_meshes, 3, 0)
	);
}