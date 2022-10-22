#pragma once

#include "Resource/Resource.h"

#include "Graphics/RHI/RHI_Buffer.h"
#include "Graphics/Mesh.h"

namespace Insight
{
	namespace Graphics
	{
		class RHI_CommandList;
	}

	namespace Runtime
	{
		class AssimpLoader;

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
		class IS_RUNTIME Mesh : public IResource
		{
			REGISTER_RESOURCE(Mesh);
		public:
			Mesh();
			virtual ~Mesh() override;

			void Draw(Graphics::RHI_CommandList* cmd_list, u32 lod_index = 0);

			/// @brief Return the transform from orgin from the model file.
			/// @return glm::mat4
			glm::mat4 GetTransform() const;

			static const u32 s_LOD_Count = 3;

		private:
			/// @brief Handle loading the resource from disk.
			/// @param file_path 
			virtual void Load();

			/// @brief Handle loading a resource from memory. This is called for a resource which is a "sub resource" of another one
			/// and exists inside another resource's disk file.
			/// @param file_path 
			virtual void LoadFromMemory(const void* data, u64 size_in_bytes);

			/// @brief Handle unloading the resource from memory.
			virtual void UnLoad();

		private:
			std::vector<MeshLOD> m_lods;

			//u32 m_vertex_offset = 0;
			//u32 m_vertex_count = 0;
			//u32 m_first_index = 0;
			//u32 m_index_count = 0;
			//
			//Graphics::RHI_Buffer* m_vertex_buffer = nullptr;
			//Graphics::RHI_Buffer* m_index_buffer = nullptr;

			std::string m_mesh_name;
			/// @brief Transform offset from the imported model.
			glm::mat4 m_transform_offset = glm::mat4(1.0f);

			friend class AssimpLoader;
		};
	}
}