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

			bool reloadAllShaders = false;
			if (ImGui::Button("ReloadAll###ReloadAllShaders"))
			{
				reloadAllShaders = true;
			}

			ImGui::Text("Shaders");
			if (ImGui::BeginTable("##ShaderWindowTable", 2))
			{
				for (size_t shaderIdx = 0; shaderIdx < shaders.size(); ++shaderIdx)
				{
					ImGui::TableNextColumn();

					Graphics::RHI_Shader* shader = shaders[shaderIdx];
					ImGui::Text("%s", shader->GetDesc().ShaderName.c_str());

					ImGui::TableNextColumn();

					std::string label = "Reload###" + shader->GetDesc().ShaderName;
					if (ImGui::Button(label.c_str()) || reloadAllShaders)
					{
						Graphics::RenderContext::Instance().GetShaderManager().ReloadShader(shader);
					}
				}

				ImGui::EndTable();
			}
		}
	}
}