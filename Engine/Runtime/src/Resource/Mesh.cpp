#include "Resource/Mesh.h"

#include "Graphics/RenderContext.h"
#include "Graphics/RHI/RHI_CommandList.h"

#include "Core/Logger.h"

namespace Insight
{
	namespace Runtime
	{
		Mesh::Mesh()
		{
		}

		Mesh::~Mesh()
		{
#if VERTEX_SPLIT_STREAMS
			Renderer::FreeVertexBuffer(m_lods[0].VertexBuffers.Position);
			Renderer::FreeVertexBuffer(m_lods[0].VertexBuffers.Normal);
			Renderer::FreeVertexBuffer(m_lods[0].VertexBuffers.Colour);
			Renderer::FreeVertexBuffer(m_lods[0].VertexBuffers.UV);
			Renderer::FreeVertexBuffer(m_lods[0].VertexBuffers.BoneIds);
			Renderer::FreeVertexBuffer(m_lods[0].VertexBuffers.BoneWeights);
			m_lods[0].VertexBuffers = {};
#else
			Renderer::FreeVertexBuffer(m_lods[0].VertexBuffer);
			m_lods[0].VertexBuffer = nullptr;
#endif
			Renderer::FreeIndexBuffer(m_lods[0].IndexBuffer);
			m_lods[0].IndexBuffer = nullptr;

			/*
			Renderer::FreeIndexBuffer(m_lods[0].IndexBuffers[0]);
			m_lods[0].IndexBuffers[0] = nullptr;
			*/
		}

		//IS_SERIALISABLE_CPP(Mesh)

		void Mesh::Draw(Graphics::RHI_CommandList* cmd_list, const u32 lod_index) const
		{
			ASSERT(lod_index < s_MAX_LOD_COUNT);
			const MeshLOD& meshLOD = m_lods[lod_index];

#if VERTEX_SPLIT_STREAMS
#else
			cmd_list->SetVertexBuffer(meshLOD.VertexBufferView);
			for (u32 i = 0; i < renderMeshLod.IndexBufferViews.size(); ++i)
			{
				cmdList->SetIndexBuffer(renderMeshLod.IndexBufferViews[i], Runtime::Mesh::kMeshIndexType);
				cmdList->DrawIndexed(renderMeshLod.Index_count, 1, renderMeshLod.First_index, renderMeshLod.Vertex_offset, 0);
			}
#endif
		}

		const std::string_view Mesh::GetName() const
		{
			return m_mesh_name;
		}

		Maths::Matrix4 Mesh::GetTransform() const
		{
			return m_transform_offset;
		}

		void Mesh::SetMaterial(Ref<MaterialAsset> material)
		{
			m_materialAsset = material;
		}

		Ref<MaterialAsset> Mesh::GetMaterialAsset() const
		{
			return m_materialAsset;
		}

		Graphics::BoundingBox Mesh::GetBoundingBox() const
		{
			return m_boundingBox;
		}

		const AssetInfo* Mesh::GetAssetInfo() const
		{
			return m_assetInfo;
		}

		const MeshLOD& Mesh::GetLOD(const u32 lodIndex) const
		{
			ASSERT(lodIndex >= 0 && lodIndex < s_MAX_LOD_COUNT);
			return m_lods[lodIndex];
		}

		u32 Mesh::GetLODCount() const
		{
			return s_MAX_LOD_COUNT;
		}
	}
}