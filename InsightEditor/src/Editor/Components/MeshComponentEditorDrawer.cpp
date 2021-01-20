#include "ispch.h"
#include "MeshComponentEditorDrawer.h"
#include "Insight/Component/MeshComponent.h"
#include "Insight/Library/ModelLibrary.h"

EDITOR_DEF_TYPE(MeshComponent, Editor::MeshComponentEditorDrawer);

	namespace Editor
	{
		void MeshComponentEditorDrawer::OnDraw(Object& obj)
		{
			MeshComponent& component = dynamic_cast<MeshComponent&>(obj);

			if (UIHelper::DrawString("Mesh path", &component.GetMeshName()))
			{
				auto modelPtr = Library::ModelLibrary::Instance()->GetAssetFromPath(component.GetMeshName());
				if (modelPtr)
				{
					component.SetMesh(modelPtr->GetMesh());
				}
			}
		}
	}