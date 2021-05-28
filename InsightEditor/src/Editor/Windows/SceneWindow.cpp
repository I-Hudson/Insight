#include "Editor/Windows/SceneWindow.h"
#include "Engine/Core/Application.h"
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
		, m_backBufferSource(-1)
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
	
//		Module::GraphicsModule::Instance()->SetEditorCamera(&camera);
		m_sceneImages.resize(Graphics::RenderGraph::Instance()->GetFrameCount());
		for (auto& image : m_sceneImages)
		{
			image = nullptr;
		}

		Graphics::RenderGraph::Instance()->RegisterOnGraphBuiltFunc("SceneWindow-GetBackbufferSource", [this]()
		{
			if (ImGuiRenderer::Instance()->IsInit())
			{
				m_backBufferSource = Graphics::RenderGraph::Instance()->GetPass("MainPass").GetTextureResource("color").GetPhysicalIndex();

				Graphics::GPUImage* image = Graphics::RenderGraph::Instance()->GetPhysicalImage(m_backBufferSource);
				Graphics::GPUImageView* imageView = Graphics::RenderGraph::Instance()->GetPhysicalImageView(m_backBufferSource);
				u32 frameIndex = Graphics::RenderGraph::Instance()->GetCurrentFrameIndex();
				if (m_sceneImages.at(frameIndex) == nullptr)
				{
					m_sceneImages[frameIndex] = AddImageVulkan(image, imageView);
				}
			}
		});
	}

	SceneWindow::~SceneWindow()
	{
		m_entityManager.DestroyEntity(m_editorCamera.GetEntityID());
	}

	void SceneWindow::Update(const float& deltaTime)
	{
		if (ImGuiRenderer::Instance()->IsInit())
		{
			ImGui::Begin("Scene", 0, ImGuiWindowFlags_NoScrollbar);
			m_windowedHovered = ImGui::IsWindowHovered();

			if (m_backBufferSource != -1)
			{
				// This displayed image to the scene window will be a single frame behind. Not get, but for now 
				// should be fine.

				glm::ivec2 windowsSize = glm::ivec2((int)ImGui::GetWindowSize().x, (int)ImGui::GetWindowSize().y);
				windowsSize.y -= (int)(ImGui::GetFrameHeight() * 1.5f);
				if (windowsSize != m_windowSize)
				{
					m_windowSize = windowsSize;
					m_backBufferSource = -1;

					SceneWindowCleanImage cleanImages;
					cleanImages.Descriptors = m_sceneImages;
					cleanImages.FrameTriggered = Application::FrameCount;
					m_imagesToClean.push(cleanImages);
					for (auto& image : m_sceneImages )
					{
						image = nullptr;
					}

					EventManager::Dispatch(EventType::WindowResize, WindowResizeEvent(m_windowSize.x, m_windowSize.y));
				}
				else
				{
					u32 frameIndex = Graphics::RenderGraph::Instance()->GetCurrentFrameIndex();
					if (m_sceneImages.at(frameIndex) != nullptr)
					{
						ImGui::Image((ImTextureID)m_sceneImages.at(frameIndex), ImVec2((float)windowsSize.x, (float)windowsSize.y));
					}
				}
			}
			ImGui::End();
		}

		if (m_windowedHovered)
		{
			m_componentManager.Update(deltaTime);
		}
		CheckForImageClean();
	}

	void SceneWindow::CheckForWindowResize()
	{
	}

	void SceneWindow::CheckForImageClean()
	{
		if (m_imagesToClean.size() == 0)
		{
			return;
		}

		SceneWindowCleanImage image = m_imagesToClean.front();
		if (Application::FrameCount > image.FrameTriggered + (u64)3)
		{
			for (auto& des : image.Descriptors)
			{
				FreeImageVulkan(des);
			}
			m_imagesToClean.pop();
		}
	}
}
#endif