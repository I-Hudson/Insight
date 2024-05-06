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
			Renderer::FreeVertexBuffer(m_lods.at(0).Vertex_buffer);
			Renderer::FreeIndexBuffer(m_lods.at(0).Index_buffer);
			m_lods.at(0).Vertex_buffer = nullptr;
			m_lods.at(0).Index_buffer = nullptr;
		}

		//IS_SERIALISABLE_CPP(Mesh)

		void Mesh::Draw(Graphics::RHI_CommandList* cmd_list, u32 lod_index)
		{
			lod_index = std::min(lod_index, static_cast<u32>(m_lods.size()));
			cmd_list->SetVertexBuffer(m_lods.at(lod_index).Vertex_buffer);
			cmd_list->SetIndexBuffer(m_lods.at(lod_index).Index_buffer, Graphics::IndexType::Uint32);
			cmd_list->DrawIndexed(m_lods.at(lod_index).Index_count, 1, m_lods.at(lod_index).First_index, m_lods.at(lod_index).Vertex_offset, 0);
		}

		const std::string_view Mesh::GetName() const
		{
			return m_mesh_name;
		}

		glm::mat4 Mesh::GetTransform() const
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
	}
}