#include "ispch.h"
#include "VulkanRendererEditorOverlay.h"

#if defined(IS_EDITOR)
#include "VulkanRenderer.h"
#include "Insight/Component/CameraComponent.h"
#include "Insight/Editor/UIHelper.h"

namespace vks
{
	VulkanRendererEditorOverlay::VulkanRendererEditorOverlay(const Insight::Module::EditorModule* editorModule, VulkanRenderer* renderer)
		: EditorPanel(editorModule)
		, m_renderer(*renderer)
	{
	}

	VulkanRendererEditorOverlay::~VulkanRendererEditorOverlay()
	{
	}

	void VulkanRendererEditorOverlay::Update(const float& deltaTime)
	{
#if defined(IS_EDITOR)
		ImGui::Begin("VulkanRendererEditorOverlay");
		
		ImGui::Text("EditorCamera");

		float fov = m_renderer.m_editorCamera->GetFov();
		float nearPlane = m_renderer.m_editorCamera->GetNearPlane();
		float farPlane = m_renderer.m_editorCamera->GetFarPlane();
		if (DrawFloat("FOV", &fov) || DrawFloat("Near Plane", &nearPlane) || DrawFloat("Far Plane", &farPlane))
		{
			m_renderer.m_editorCamera->SetProjMatrix(fov, nearPlane, farPlane);
		}

		ImGui::End();
#endif
	}
}
#endif