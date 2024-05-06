#pragma once

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
		class ModelImporter;

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
		class IS_RUNTIME Mesh
		{
			REFLECT_GENERATED_BODY()
		
		public:
			Mesh();
			~Mesh();

			//IS_SERIALISABLE_H(Mesh)

			void Draw(Graphics::RHI_CommandList* cmd_list, u32 lod_index = 0);

			const std::string_view GetName() const;

			/// @brief Return the transform from orgin from the model file.
			/// @return glm::mat4
			glm::mat4 GetTransform() const;

			void SetMaterial(Ref<MaterialAsset> material);
			Ref<MaterialAsset> GetMaterialAsset() const;

			Graphics::BoundingBox GetBoundingBox() const;

			const AssetInfo* GetAssetInfo() const;

			static const u32 s_LOD_Count = 4;

		private:
			std::vector<MeshLOD> m_lods;
			Ref<MaterialAsset> m_materialAsset = nullptr;
			Graphics::BoundingBox m_boundingBox;

			std::string m_mesh_name;
			/// @brief Transform offset from the imported model.
			glm::mat4 m_transform_offset = glm::mat4(1.0f);

			// If this mesh was loaded from an asset on disk then this will be valid.
			const AssetInfo* m_assetInfo = nullptr;

			friend class ModelImporter;
			friend struct RenderMesh;
		};
	}

	//OBJECT_SERIALISER(Runtime::Mesh, 1,
	//	SERIALISE_BASE(Runtime::IResource, 1, 0)
	//);
}