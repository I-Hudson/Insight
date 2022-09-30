#pragma once

#include "Resource/Resource.h"


namespace Insight
{
	namespace Runtime
	{
		class Mesh;

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
			// std::vector<Materials> m_materials
		};
	}
}