#pragma once

#include "Resource/Resource.h"

#include "Graphics/RHI/RHI_Buffer.h"

namespace Insight
{
	namespace Runtime
	{
		class Mesh;
		class AssimpLoader;

		class Model : public IResource
		{
			REGISTER_RESOURCE(Model);
		public:

			Mesh* GetMesh() const;
			Mesh* GetMeshByIndex(u32 index) const;

		private:
			/// @brief Handle loading the resource from disk.
			/// @param file_path 
			virtual void Load() override;
			/// @brief Handle unloading the resource from memory.
			virtual void UnLoad() override;

		private:
			std::vector<Mesh*> m_meshes;
			Graphics::RHI_Buffer* m_vertex_buffer = nullptr;
			Graphics::RHI_Buffer* m_index_buffer = nullptr;
			// std::vector<Materials> m_materials

			friend class AssimpLoader;
		};
	}
}