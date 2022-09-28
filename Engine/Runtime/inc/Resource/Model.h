#pragma once

#include "Resource/Resource.h"

#include "Graphics/Mesh.h"

namespace Insight
{
	namespace Runtime
	{
		class Model : public IResource
		{
			REGISTER_RESOURCE(Model);
		public:

			Ptr<Graphics::Mesh> GetMesh() const;

		private:
			/// @brief Handle loading the resource from disk.
			/// @param file_path 
			virtual void Load() override;
			/// @brief Handle unloading the resource from memory.
			virtual void UnLoad() override;

		private:
			UPtr<Graphics::Mesh> m_mesh;
			// std::vector<Materials> m_materials
		};
	}
}