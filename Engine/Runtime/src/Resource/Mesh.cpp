#include "Resource/Mesh.h"
#include "Resource/Loaders/ModelLoader.h"

#include "Graphics/RenderContext.h"
#include "Graphics/RHI/RHI_CommandList.h"

#include "Core/Logger.h"

namespace Insight
{
	namespace Runtime
	{
		Mesh::Mesh()
			: IResource("")
		{
			m_lods.push_back(MeshLOD());
		}

		Mesh::Mesh(std::string_view filePath)
			: IResource(filePath)
		{
			m_lods.push_back(MeshLOD());
		}

		Mesh::~Mesh()
		{

		}

		IS_SERIALISABLE_CPP(Mesh)

		void Mesh::Draw(Graphics::RHI_CommandList* cmd_list, u32 lod_index)
		{
			lod_index = std::min(lod_index, static_cast<u32>(m_lods.size()));
			cmd_list->SetVertexBuffer(m_lods.at(lod_index).Vertex_buffer);
			cmd_list->SetIndexBuffer(m_lods.at(lod_index).Index_buffer, Graphics::IndexType::Uint32);
			cmd_list->DrawIndexed(m_lods.at(lod_index).Index_count, 1, m_lods.at(lod_index).First_index, m_lods.at(lod_index).Vertex_offset, 0);
		}

		glm::mat4 Mesh::GetTransform() const
		{
			return m_transform_offset;
		}

		void Mesh::SetMaterial(Material* material)
		{
			m_material = material;
		}

		Material* Mesh::GetMaterial() const
		{
			return m_material;
		}

		Graphics::BoundingBox Mesh::GetBoundingBox() const
		{
			return m_boundingBox;
		}

		void Mesh::UnLoad()
		{
			//ASSERT(m_vertex_buffer);
			//ASSERT(m_index_buffer);
			if (std::find_if(m_reference_links.begin(), m_reference_links.end(), [](const ResourceReferenceLink& link)
				{
					return link.GetReferenceLinkType() == ResourceReferenceLinkType::Dependent;
				}) == m_reference_links.end())
			{
				Renderer::FreeVertexBuffer(m_lods.at(0).Vertex_buffer);
				Renderer::FreeIndexBuffer(m_lods.at(0).Index_buffer);
				m_lods.at(0).Vertex_buffer = nullptr;
				m_lods.at(0).Index_buffer = nullptr;
			}
			m_lods.clear();
		}
	}
}