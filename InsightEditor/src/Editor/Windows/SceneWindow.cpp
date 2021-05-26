#include "Editor/Windows/SceneWindow.h"
#include "Engine/Component/TransformComponent.h"
#include "Engine/Component/CameraComponent.h"
#include "Engine/Module/GraphicsModule.h"

#include "Engine/Graphics/RenderGraph/RenderGraph.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanUtils.h"
#include "Engine/Event/EventManager.h"
#include "Engine/Event/ApplicationEvent.h"

#ifdef IS_EDITOR
namespace Insight::Editor
{
	SceneWindow::SceneWindow(const Module::EditorModule* editorModule)
		: EditorWindow(editorModule)
		, m_entityManager(EntityManager(nullptr, m_componentManager))
		, m_windowSize(glm::ivec2(1280, 720))
	{
		SET_PANEL_NAME(SceneWindow);

		m_editorCamera = m_entityManager.CreateEntity();
		m_editorCamera.AddComponent<TransformComponent>();
		m_editorCamera.SetName("New Test Entity");
		m_editorCamera.GetComponent<TransformComponent>().SetPosition(glm::vec3(0.0f, -0.0f, 10.0f));
		//m_editorCamera.GetComponent<TransformComponent>().SetRotation(glm::vec3(-15.0f, -390.0f, 0.0f));
		//glm::vec3 rot = m_editorCamera.GetComponent<TransformComponent>().GetRotation();

		CameraComponent& camera = m_editorCamera.AddComponent<CameraComponent>();
		camera.SetProjMatrix(90.0f, 0.1f, 10000.0f);
		camera.SetCameraSpeed(25.0f);
	
		::Module::GraphicsModule::Instance()->SetEditorCamera(&camera);
	}

	SceneWindow::~SceneWindow()
	{
		m_entityManager.DestroyEntity(m_editorCamera.GetEntityID());
	}

	void SceneWindow::Update(const float& deltaTime)
	{
		CheckForWindowResize();
		if (IsMouseInBounds())
		{
			m_componentManager.Update(deltaTime);
		}

		if (ImGuiRenderer::Instance()->IsInit())
		{
			ImGui::Begin("Scene");
			u32 physicalIndex = Graphics::RenderGraph::Instance()->GetPass("MainPass").GetTextureResource("color").GetPhysicalIndex();
			Graphics::GPUImage* image = Graphics::RenderGraph::Instance()->GetPass("MainPass").GetPhysicalImage(physicalIndex);
			Graphics::GPUImageView* imageView = Graphics::RenderGraph::Instance()->GetPass("MainPass").GetPhysicalImageView(physicalIndex);
			glm::ivec2 windowsSize = glm::ivec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);
			ImGui::Image((ImTextureID)AddImageVulkan(image, imageView), ImVec2(windowsSize.x, windowsSize.y));
			ImGui::End();
			
			if (windowsSize != m_windowSize)
			{
				m_windowSize = windowsSize;
				EventManager::Dispatch(EventType::WindowResize, WindowResizeEvent(m_windowSize.x, m_windowSize.y));
			}
		}
	}

	void SceneWindow::CheckForWindowResize()
	{
	}
}
#endif