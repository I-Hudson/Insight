#pragma once

#include "Resource/Resource.h"

#include "Graphics/RHI/RHI_Buffer.h"
#include "Graphics/BoundingBox.h"

#include "Asset/Assets/Material.h"

#include "Generated/Mesh_reflect_generated.h"

namespace Insight
{
	struct RenderMesh;
	namespace Graphics
	{
		class RHI_CommandList;
	}

	namespace Runtime
	{
		class Material;
		class ModelImporter;
		class ModelLoader;
		class Model;

		/// @brief Define the data used to create a mesh from a model.
		struct MeshFromModelData
		{
			std::vector<Graphics::Vertex> Vertices;
			std::vector<u32> Indcies;
		};

		struct MeshLOD
		{
			u32 LOD_index = 0;
			u32 Vertex_offset = 0;
			u32 Vertex_count = 0;
			u32 First_index = 0;
			u32 Index_count = 0;

			Graphics::RHI_Buffer* Vertex_buffer = nullptr;
			Graphics::RHI_Buffer* Index_buffer = nullptr;
		};

		/// @brief Contain vertex and index buffers for use when rendering this mesh.
		/// This class stores only geometry data.
		REFLECT_CLASS()
		class IS_RUNTIME Mesh : public IResource
		{
			REFLECT_GENERATED_BODY()
		
		public:
			Mesh();
			Mesh(std::string_view filePath);
			virtual ~Mesh() override;

			REGISTER_RESOURCE(Mesh);
			IS_SERIALISABLE_H(Mesh)

			void Draw(Graphics::RHI_CommandList* cmd_list, u32 lod_index = 0);

			const std::string_view GetName() const;

			/// @brief Return the transform from orgin from the model file.
			/// @return glm::mat4
			glm::mat4 GetTransform() const;
			
			void SetMaterial(Material* material);
			Material* GetMaterial() const;

			void SetMaterial(Ref<MaterialAsset> material);
			Ref<MaterialAsset> GetMaterialAsset() const;

			Graphics::BoundingBox GetBoundingBox() const;

			static const u32 s_LOD_Count = 4;

		private:
			/// @brief Handle unloading the resource from memory.
			virtual void UnLoad() override final;

		private:
			std::vector<MeshLOD> m_lods;
			Material* m_material = nullptr;
			Ref<MaterialAsset> m_materialAsset = nullptr;
			Graphics::BoundingBox m_boundingBox;

			std::string m_mesh_name;
			/// @brief Transform offset from the imported model.
			glm::mat4 m_transform_offset = glm::mat4(1.0f);

			friend class ModelLoader;
			friend class ModelImporter;
			friend struct RenderMesh;
			friend class Model;
		};
	}

	OBJECT_SERIALISER(Runtime::Mesh, 1,
		SERIALISE_BASE(Runtime::IResource, 1, 0)
	);
}