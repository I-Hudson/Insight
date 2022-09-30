#include "Resource/Model.h"
#include "Resource/Mesh.h"
#include "Resource/Texture.h"
#include "Resource/Loaders/AssimpLoader.h"

#include <filesystem>

namespace Insight
{
	namespace Runtime
	{
		Mesh* Model::GetMesh() const
		{
			return GetMeshByIndex(0);
		}

		Mesh* Model::GetMeshByIndex(u32 index) const
		{
			return m_meshes.at(index);
		}

		void Model::Load()
		{
			ASSERT(m_meshes.size() == 0);
			if (!AssimpLoader::LoadModel(this, m_file_path, AssimpLoader::Default_Model_Importer_Flags))
			{
				m_resource_state = EResoruceStates::Failed_To_Load;
				return;
			}
			m_resource_state = EResoruceStates::Loaded;
		}

		void Model::UnLoad()
		{
			// Unload all our memory meshes.
			for (Mesh* mesh : m_meshes)
			{
				ResourceManager::Instance().Unload(mesh);
			}
			m_meshes.clear();
			m_resource_state = EResoruceStates::Unloaded;
		}
	}
}