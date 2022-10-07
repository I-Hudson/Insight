#include "Resource/Mesh.h"
#include "Resource/Loaders/AssimpLoader.h"

#include "Graphics/RenderContext.h"
#include "Graphics/RHI/RHI_CommandList.h"

#include "Core/Logger.h"

namespace Insight
{
	namespace Runtime
	{
		void Mesh::Draw(Graphics::RHI_CommandList* cmd_list)
		{
			cmd_list->SetVertexBuffer(m_vertex_buffer);
			cmd_list->SetIndexBuffer(m_index_buffer, Graphics::IndexType::Uint32);
			cmd_list->DrawIndexed(m_index_count, 1, m_first_index, m_vertex_offset, 0);
		}

		glm::mat4 Mesh::GetTransform() const
		{
			return m_transform_offset;
		}

		void Mesh::Load()
		{
			ASSERT(!m_vertex_buffer);
			ASSERT(!m_index_buffer);

			// Loading this mesh from a file. Only import mesh information.
			// If loading a multi-part mesh with the mesh class then the mesh will be collapsed down to a single mesh.
			// It is recomend to always load a "Model" instead due to this. "Model" will kepp the hierarchy "Mesh" will not. 
		
			if (!AssimpLoader::LoadMesh(this, m_file_path, AssimpLoader::Default_Mesh_Importer_Flags))
			{
				m_resource_state = EResoruceStates::Failed_To_Load;
			}
			m_resource_state = EResoruceStates::Loaded;
		}

		void Mesh::LoadFromMemory(const void* data, u64 size_in_bytes)
		{
			ASSERT(!m_vertex_buffer);
			ASSERT(!m_index_buffer);

			// Could be loading a mesh from a file or model.
			MeshFromModelData const* mesh_data = static_cast<const MeshFromModelData*>(data);
			if (!mesh_data)
			{
				IS_CORE_ERROR("[Mesh::LoadFromMemory] Unable to load mesh from memory. 'MeshFromModelData' cast failed.");
				return;
			}

			u64 const vertex_buffer_size = sizeof(Graphics::Vertex) * mesh_data->Vertices.size();
			u64 const index_buffer_size = sizeof(u32) * mesh_data->Indcies.size();

			m_vertex_buffer = Renderer::CreateVertexBuffer(vertex_buffer_size, sizeof(Graphics::Vertex));
			m_index_buffer = Renderer::CreateIndexBuffer(index_buffer_size);
			m_vertex_buffer->Upload(mesh_data->Vertices.data(), vertex_buffer_size);
			m_index_buffer->Upload(mesh_data->Indcies.data(), index_buffer_size);
		}

		void Mesh::UnLoad()
		{
			ASSERT(m_vertex_buffer);
			ASSERT(m_index_buffer);
			if (std::find_if(m_reference_links.begin(), m_reference_links.end(), [](const ResourceReferenceLink& link)
				{
					return link.GetReferenceLinkType() == ResourceReferenceLinkType::Dependent;
				}) == m_reference_links.end())
			{
				Renderer::FreeVertexBuffer(m_vertex_buffer);
				Renderer::FreeIndexBuffer(m_index_buffer);
			}
			else
			{
				m_vertex_buffer = nullptr;
				m_index_buffer = nullptr;
			}
		}
	}
}