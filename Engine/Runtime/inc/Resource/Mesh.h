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
		/// @brief Define the data used to create a mesh from a model.
		struct MeshFromModelData
		{
			std::vector<Graphics::Vertex> Vertices;
			std::vector<u32> Indcies;
		};

		/// @brief Contain vertex and index buffers for use when rendering this mesh.
		/// This class stores only geometry data.
		class IS_RUNTIME Mesh : public IResource
		{
			REGISTER_RESOURCE(Mesh);
		public:

			void Draw(Graphics::RHI_CommandList* cmd_list);

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
			u32 m_vertex_count = 0;
			u32 m_index_count = 0;

			Graphics::RHI_Buffer* m_vertex_buffer = nullptr;
			Graphics::RHI_Buffer* m_index_buffer = nullptr;
		};
	}
}