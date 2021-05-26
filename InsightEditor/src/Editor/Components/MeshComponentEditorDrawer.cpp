#include "ispch.h"
#include "Editor/Components/MeshComponentEditorDrawer.h"
#include "Engine/Component/MeshComponent.h"
#include "Engine/Library/ModelLibrary.h"

EDITOR_DEF_TYPE(MeshComponent, Insight::Editor::MeshComponentEditorDrawer);

namespace Insight::Editor
{
	void MeshComponentEditorDrawer::OnDraw(Object& obj)
	{
		MeshComponent& component = dynamic_cast<MeshComponent&>(obj);

		if (UIHelper::DrawString("Mesh path", &component.GetMeshName()))
		{
			auto modelPtr = Library::ModelLibrary::Instance()->GetAssetFromPath(component.GetMeshName());
			if (modelPtr)
			{
				//component.SetMesh(modelPtr->GetMesh());
			}
		}
	}
}