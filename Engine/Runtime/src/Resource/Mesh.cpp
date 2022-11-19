#include "Resource/Mesh.h"
#include "Resource/Loaders/AssimpLoader.h"

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

		}

		void Mesh::Draw(Graphics::RHI_CommandList* cmd_list, u32 lod_index)
		{
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

		void Mesh::Load()
		{
			//ASSERT(!m_vertex_buffer);
			//ASSERT(!m_index_buffer);

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
			//ASSERT(!m_vertex_buffer);
			//ASSERT(!m_index_buffer);

			// Could be loading a mesh from a file or model.
			MeshFromModelData const* mesh_data = static_cast<const MeshFromModelData*>(data);
			if (!mesh_data)
			{
				IS_CORE_ERROR("[Mesh::LoadFromMemory] Unable to load mesh from memory. 'MeshFromModelData' cast failed.");
				return;
			}

			u64 const vertex_buffer_size = sizeof(Graphics::Vertex) * mesh_data->Vertices.size();
			u64 const index_buffer_size = sizeof(u32) * mesh_data->Indcies.size();

			m_lods.push_back(MeshLOD());
			MeshLOD& lod_0 = m_lods.back();

			lod_0.Vertex_buffer = Renderer::CreateVertexBuffer(vertex_buffer_size, sizeof(Graphics::Vertex));
			lod_0.Index_buffer = Renderer::CreateIndexBuffer(index_buffer_size);
			lod_0.Vertex_buffer->Upload(mesh_data->Vertices.data(), vertex_buffer_size);
			lod_0.Index_buffer->Upload(mesh_data->Indcies.data(), index_buffer_size);
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
			}
			m_lods.clear();
		}
	}
}