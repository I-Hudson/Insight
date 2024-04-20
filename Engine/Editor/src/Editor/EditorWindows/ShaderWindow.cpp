#include "Editor/EditorWindows/ShaderWindow.h"

#include "Graphics/RenderContext.h"

#include <imgui.h>

namespace Insight
{
	namespace Editor
	{
		ShaderWindow::ShaderWindow()
			: IEditorWindow()
		{ }
		ShaderWindow::ShaderWindow(u32 minWidth, u32 minHeight)
			: IEditorWindow(minWidth, minHeight)
		{ }
		ShaderWindow::ShaderWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight)
			: IEditorWindow(minWidth, minHeight, maxWidth, maxHeight)
		{ }
		ShaderWindow::~ShaderWindow()
		{ }

		void ShaderWindow::OnDraw()
		{
			Graphics::RHI_ShaderManager& shaderManger = Graphics::RenderContext::Instance().GetShaderManager();
			const std::vector<Graphics::RHI_Shader*> shaders = shaderManger.GetAllShaders();

			if (ImGui::BeginTable("##ShaderWindowTable", 3))
			{
				ImGui::TableSetupColumn("Name");

				ImGui::TableHeadersRow();
				ImGui::TableNextColumn();

				for (size_t shaderIdx = 0; shaderIdx < shaders.size(); ++shaderIdx)
				{
					Graphics::RHI_Shader* shader = shaders[shaderIdx];

					ImGui::Text("%s", shader->GetDesc().ShaderName.c_str());
					ImGui::TableNextColumn();
				}
				ImGui::EndTable();
			}
		}
	}
}