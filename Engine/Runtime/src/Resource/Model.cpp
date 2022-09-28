#include "Resource/Model.h"

#include <filesystem>

namespace Insight
{
	namespace Runtime
	{
		Ptr<Graphics::Mesh> Model::GetMesh() const
		{
			return m_mesh.Get();
		}

		void Model::Load()
		{
			ASSERT(m_mesh == nullptr);
			m_mesh = MakeUPtr<Graphics::Mesh>();
			if (!m_mesh->LoadFromFile(m_file_path))
			{
				m_resource_state = EResoruceState::Failed_To_Load;
				return;
			}
			m_resource_state = EResoruceState::Loaded;
		}

		void Model::UnLoad()
		{
			m_mesh->Destroy();
			m_mesh.Reset();
			m_resource_state = EResoruceState::Unloaded;
		}
	}
}