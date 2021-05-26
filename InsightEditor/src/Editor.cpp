#include <Insight.h>
#include "Engine/Config/Config.h"
#include "Module/EditorModule.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/string_cast.hpp"
#include "Engine/Graphics/Debug/Gizmos.h"
#include "ImGuizmo.h"

class EditorApp : public Application
{
public:
	Insight::Module::EditorModule* m_editorModule;

	EditorApp() : Application()
	{ }

	virtual void Create() override
	{
#ifdef IMGUI_ENABLED
		m_editorModule = m_moduleManager->AddModule<Insight::Module::EditorModule>();
		Config::GetInstance().Parse("./data/config/editorConfig.txt");

		auto& style = ImGui::GetStyle().Colors;

		glm::vec4 windowBg = Insight::Module::EditorModule::Instance()->EditorConfig.WindowBG.GetVal();
		style[ImGuiCol_WindowBg] = ImVec4(windowBg.x, windowBg.y, windowBg.z, windowBg.w);

		glm::vec4 header = Insight::Module::EditorModule::Instance()->EditorConfig.Header.GetVal();
		style[ImGuiCol_Header] = ImVec4(header.x, header.y, header.z, header.w);
		glm::vec4 headerHovered = Insight::Module::EditorModule::Instance()->EditorConfig.HeaderHovered.GetVal();
		style[ImGuiCol_HeaderHovered] = ImVec4(headerHovered.x, headerHovered.y, headerHovered.z, headerHovered.w);
		glm::vec4 headerActive = Insight::Module::EditorModule::Instance()->EditorConfig.HeaderActive.GetVal();
		style[ImGuiCol_HeaderActive] = ImVec4(headerActive.x, headerActive.y, headerActive.z, headerActive.w);

		glm::vec4 button = Insight::Module::EditorModule::Instance()->EditorConfig.Button.GetVal();
		style[ImGuiCol_Button] = ImVec4(button.x, button.y, button.z, button.w);
		glm::vec4 buttonHovered = Insight::Module::EditorModule::Instance()->EditorConfig.ButtonHovered.GetVal();
		style[ImGuiCol_ButtonHovered] = ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, buttonHovered.w);
		glm::vec4 buttonActive = Insight::Module::EditorModule::Instance()->EditorConfig.ButtonActive.GetVal();
		style[ImGuiCol_ButtonActive] = ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, buttonActive.w);

		glm::vec4 frame = Insight::Module::EditorModule::Instance()->EditorConfig.FrameBg.GetVal();
		style[ImGuiCol_FrameBg] = ImVec4(frame.x, frame.y, frame.z, frame.w);
		glm::vec4 frameBg = Insight::Module::EditorModule::Instance()->EditorConfig.FrameBgHovered.GetVal();
		style[ImGuiCol_FrameBgHovered] = ImVec4(frameBg.x, frameBg.y, frameBg.z, frameBg.w);
		glm::vec4 frameBgActive = Insight::Module::EditorModule::Instance()->EditorConfig.FrameBgActive.GetVal();
		style[ImGuiCol_FrameBgActive] = ImVec4(frameBgActive.x, frameBgActive.y, frameBgActive.z, frameBgActive.w);

		glm::vec4 tab = Insight::Module::EditorModule::Instance()->EditorConfig.Tab.GetVal();
		style[ImGuiCol_Tab] = ImVec4(tab.x, tab.y, tab.z, tab.w);
		glm::vec4 tabHovered = Insight::Module::EditorModule::Instance()->EditorConfig.TabHovered.GetVal();
		style[ImGuiCol_TabHovered] = ImVec4(tabHovered.x, tabHovered.y, tabHovered.z, tabHovered.w);
		glm::vec4 tabActive = Insight::Module::EditorModule::Instance()->EditorConfig.TabActive.GetVal();
		style[ImGuiCol_TabActive] = ImVec4(tabActive.x, tabActive.y, tabActive.z, tabActive.w);
		glm::vec4 tabUnfocused = Insight::Module::EditorModule::Instance()->EditorConfig.TabUnfocused.GetVal();
		style[ImGuiCol_TabUnfocused] = ImVec4(tabUnfocused.x, tabUnfocused.y, tabUnfocused.z, tabUnfocused.w);
		glm::vec4 tabUnfocusedActive = Insight::Module::EditorModule::Instance()->EditorConfig.TabUnfocusedActive.GetVal();
		style[ImGuiCol_TabUnfocusedActive] = ImVec4(tabUnfocusedActive.x, tabUnfocusedActive.y, tabUnfocusedActive.z, tabUnfocusedActive.w);

		glm::vec4 titleBg = Insight::Module::EditorModule::Instance()->EditorConfig.TitleBg.GetVal();
		style[ImGuiCol_TitleBg] = ImVec4(titleBg.x, titleBg.y, titleBg.z, titleBg.w);
		glm::vec4 titleBgActive = Insight::Module::EditorModule::Instance()->EditorConfig.TitleBgActive.GetVal();
		style[ImGuiCol_TitleBgActive] = ImVec4(titleBgActive.x, titleBgActive.y, titleBgActive.z, titleBgActive.w);
		glm::vec4 titleBgCollapsed = Insight::Module::EditorModule::Instance()->EditorConfig.TitleBgCollapsed.GetVal();
		style[ImGuiCol_TitleBgCollapsed] = ImVec4(titleBgCollapsed.x, titleBgCollapsed.y, titleBgCollapsed.z, titleBgCollapsed.w);
#endif

		Entity& dirLight = Scene::ActiveScene()->CreateEntity("DirectionalLight");
		dirLight.GetComponent<TransformComponent>().SetPosition({ 25, 25, 25 });
		DirectionalLightComponent& lightCom = dirLight.AddComponent<DirectionalLightComponent>();
		DirectionalLightComponentData& lightData = lightCom.GetComponentData<DirectionalLightComponentData>();
		lightData.FOV = 15.0f;
		lightData.Direction = glm::normalize(glm::vec3(0,0,0) - dirLight.GetComponent<TransformComponent>().GetPostion());
		lightData.NearPlane = 1.0f;
		lightData.FarPlane = 96.f;

		for (size_t i = 0; i < 1; ++i)
		{
			Insight::Graphics::Model* graphicsModel = FileSystem::FileSystemManager::Instance()->LoadObject<Insight::Graphics::Model>("./data/models/vulkanscene_shadow.gltf");
			//Insight::Graphics::Model* graphicsModel = FileSystem::FileSystemManager::Instance()->LoadObject<Insight::Graphics::Model>("./data/models/sponza/sponza.obj");

			Entity& mesh = Scene::ActiveScene()->CreateEntity("Mesh");
			MeshComponent& meshComponent = mesh.AddComponent<MeshComponent>();
			meshComponent.SetModel(graphicsModel);
			meshComponent.GetEntity().GetComponent<TransformComponent>().SetPosition({ 0, 0, 0 });

			Entity meshChild = Scene::ActiveScene()->CreateEntity("Mesh Entity Child");
			meshChild.SetParent(mesh.GetEntityID());
		}

		::New<Insight::Graphics::Debug::Gizmos>();
	}

	virtual void Update(const float deltaTime) override
	{ }

	virtual void Draw() override
	{ }

	virtual void OnFrameEnd() override
	{ }

	~EditorApp()
	{
		::Delete(Insight::Graphics::Debug::Gizmos::Instance());
	}
};

Application* CreateApplication()
{
	return new EditorApp();
}