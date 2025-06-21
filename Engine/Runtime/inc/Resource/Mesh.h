#pragma once

#include "Graphics/RHI/RHI_Buffer.h"
#include "Graphics/BoundingBox.h"

#include "Asset/Assets/Material.h"

#include "Graphics/Vertex.h"

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

#ifdef VERTEX_SPLIT_STREAMS
			Graphics::VerticesSplitRHIBuffers VertexBuffers;
#else
			Graphics::RHI_Buffer* VertexBuffer = nullptr;
			Graphics::RHI_BufferView VertexBufferView;
#endif
			Graphics::RHI_Buffer* IndexBuffer = nullptr;
			Graphics::RHI_BufferView IndexBufferView;
		};

		/// @brief Contain vertex and index buffers for use when rendering this mesh.
		/// This class stores only geometry data.
		REFLECT_CLASS()
		class IS_RUNTIME Mesh : public Core::RefCount
		{
			REFLECT_GENERATED_BODY()
		
		public:
			Mesh();
			~Mesh();

			//IS_SERIALISABLE_H(Mesh)

			void Draw(Graphics::RHI_CommandList* cmd_list, const u32 lod_index = 0) const;

			const std::string_view GetName() const;

			/// @brief Return the transform from orgin from the model file.
			/// @return Maths::Matrix4
			Maths::Matrix4 GetTransform() const;

			void SetMaterial(Ref<MaterialAsset> material);
			Ref<MaterialAsset> GetMaterialAsset() const;

			Graphics::BoundingBox GetBoundingBox() const;

			const AssetInfo* GetAssetInfo() const;

			const MeshLOD& GetLOD(const u32 lodIndex = 0) const;
			u32 GetLODCount() const;
			static const u32 s_MAX_LOD_COUNT = 4;

		private:
			std::vector<MeshLOD> m_lods;
			Ref<MaterialAsset> m_materialAsset = nullptr;
			Graphics::BoundingBox m_boundingBox;

			std::string m_mesh_name;
			/// @brief Transform offset from the imported model.
			Maths::Matrix4 m_transform_offset = Maths::Matrix4::Identity;

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