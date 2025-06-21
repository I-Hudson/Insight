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
			m_lods.push_back(MeshLOD());
		}

		Mesh::~Mesh()
		{
#ifdef VERTEX_SPLIT_STREAMS
			Renderer::FreeVertexBuffer(m_lods.at(0).VertexBuffers.Position);
			Renderer::FreeVertexBuffer(m_lods.at(0).VertexBuffers.Normal);
			Renderer::FreeVertexBuffer(m_lods.at(0).VertexBuffers.Colour);
			Renderer::FreeVertexBuffer(m_lods.at(0).VertexBuffers.UV);
			Renderer::FreeVertexBuffer(m_lods.at(0).VertexBuffers.BoneIds);
			Renderer::FreeVertexBuffer(m_lods.at(0).VertexBuffers.BoneWeights);
#else
			Renderer::FreeVertexBuffer(m_lods.at(0).VertexBuffer);
			m_lods.at(0).VertexBuffer = nullptr;
#endif
			Renderer::FreeIndexBuffer(m_lods.at(0).IndexBuffer);
			m_lods.at(0).IndexBuffer = nullptr;
		}

		//IS_SERIALISABLE_CPP(Mesh)

		void Mesh::Draw(Graphics::RHI_CommandList* cmd_list, const u32 lod_index) const
		{
			const u32 lodIndex = std::min(lod_index, static_cast<u32>(m_lods.size()));
			const MeshLOD& meshLOD = m_lods[lodIndex];

#ifdef VERTEX_SPLIT_STREAMS
#else
			cmd_list->SetVertexBuffer(meshLOD.VertexBufferView);
			cmd_list->SetIndexBuffer(meshLOD.IndexBufferView, Graphics::IndexType::Uint32);
			cmd_list->DrawIndexed(meshLOD.Index_count, 1, meshLOD.First_index, meshLOD.Vertex_offset, 0);
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
			ASSERT(lodIndex >= 0 && lodIndex < m_lods.size());
			return m_lods[lodIndex];
		}

		u32 Mesh::GetLODCount() const
		{
			return static_cast<u32>(m_lods.size());
		}
	}
}