#include <Insight.h>
#include "Insight/Config/Config.h"
#include "Module/EditorModule.h"

class EditorApp : public Application
{
public:
	SharedPtr<Module::EditorModule> m_editorModule;

	EditorApp() : Application()
	{ }

	virtual void Create() override
	{
		m_editorModule = m_moduleManager->AddModule<Module::EditorModule>();
		Config::GetInstance().Parse("./data/config/editorConfig.txt");

		auto& style = ImGui::GetStyle().Colors;

		glm::vec4 windowBg = Module::EditorModule::Instance()->EditorConfig.WindowBG.GetVal();
		style[ImGuiCol_WindowBg] = ImVec4(windowBg.x, windowBg.y, windowBg.z, windowBg.w);

		glm::vec4 header = Module::EditorModule::Instance()->EditorConfig.Header.GetVal();
		style[ImGuiCol_Header] = ImVec4(header.x, header.y, header.z, header.w);
		glm::vec4 headerHovered = Module::EditorModule::Instance()->EditorConfig.HeaderHovered.GetVal();
		style[ImGuiCol_HeaderHovered] = ImVec4(headerHovered.x, headerHovered.y, headerHovered.z, headerHovered.w);
		glm::vec4 headerActive = Module::EditorModule::Instance()->EditorConfig.HeaderActive.GetVal();
		style[ImGuiCol_HeaderActive] = ImVec4(headerActive.x, headerActive.y, headerActive.z, headerActive.w);

		glm::vec4 button = Module::EditorModule::Instance()->EditorConfig.Button.GetVal();
		style[ImGuiCol_Button] = ImVec4(button.x, button.y, button.z, button.w);
		glm::vec4 buttonHovered = Module::EditorModule::Instance()->EditorConfig.ButtonHovered.GetVal();
		style[ImGuiCol_ButtonHovered] = ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, buttonHovered.w);
		glm::vec4 buttonActive = Module::EditorModule::Instance()->EditorConfig.ButtonActive.GetVal();
		style[ImGuiCol_ButtonActive] = ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, buttonActive.w);

		glm::vec4 frame = Module::EditorModule::Instance()->EditorConfig.FrameBg.GetVal();
		style[ImGuiCol_FrameBg] = ImVec4(frame.x, frame.y, frame.z, frame.w);
		glm::vec4 frameBg = Module::EditorModule::Instance()->EditorConfig.FrameBgHovered.GetVal();
		style[ImGuiCol_FrameBgHovered] = ImVec4(frameBg.x, frameBg.y, frameBg.z, frameBg.w);
		glm::vec4 frameBgActive = Module::EditorModule::Instance()->EditorConfig.FrameBgActive.GetVal();
		style[ImGuiCol_FrameBgActive] = ImVec4(frameBgActive.x, frameBgActive.y, frameBgActive.z, frameBgActive.w);

		glm::vec4 tab = Module::EditorModule::Instance()->EditorConfig.Tab.GetVal();
		style[ImGuiCol_Tab] = ImVec4(tab.x, tab.y, tab.z, tab.w);
		glm::vec4 tabHovered = Module::EditorModule::Instance()->EditorConfig.TabHovered.GetVal();
		style[ImGuiCol_TabHovered] = ImVec4(tabHovered.x, tabHovered.y, tabHovered.z, tabHovered.w);
		glm::vec4 tabActive = Module::EditorModule::Instance()->EditorConfig.TabActive.GetVal();
		style[ImGuiCol_TabActive] = ImVec4(tabActive.x, tabActive.y, tabActive.z, tabActive.w);
		glm::vec4 tabUnfocused = Module::EditorModule::Instance()->EditorConfig.TabUnfocused.GetVal();
		style[ImGuiCol_TabUnfocused] = ImVec4(tabUnfocused.x, tabUnfocused.y, tabUnfocused.z, tabUnfocused.w);
		glm::vec4 tabUnfocusedActive = Module::EditorModule::Instance()->EditorConfig.TabUnfocusedActive.GetVal();
		style[ImGuiCol_TabUnfocusedActive] = ImVec4(tabUnfocusedActive.x, tabUnfocusedActive.y, tabUnfocusedActive.z, tabUnfocusedActive.w);

		glm::vec4 titleBg = Module::EditorModule::Instance()->EditorConfig.TitleBg.GetVal();
		style[ImGuiCol_TitleBg] = ImVec4(titleBg.x, titleBg.y, titleBg.z, titleBg.w);
		glm::vec4 titleBgActive = Module::EditorModule::Instance()->EditorConfig.TitleBgActive.GetVal();
		style[ImGuiCol_TitleBgActive] = ImVec4(titleBgActive.x, titleBgActive.y, titleBgActive.z, titleBgActive.w);
		glm::vec4 titleBgCollapsed = Module::EditorModule::Instance()->EditorConfig.TitleBgCollapsed.GetVal();
		style[ImGuiCol_TitleBgCollapsed] = ImVec4(titleBgCollapsed.x, titleBgCollapsed.y, titleBgCollapsed.z, titleBgCollapsed.w);
	
		for (size_t i = 0; i < 4; i++)
		{
			auto meshComponent = Entity::Create("Nano suit Entity")->AddComponent<MeshComponent>();
			auto model = FileSystem::FileSystemManager::Instance()->LoadObject<Model>("./data/models/nano/nanosuit.fbx");
			meshComponent->SetModel(model);
			meshComponent->GetEntity().lock()->GetComponent<TransformComponent>()->SetPosition(glm::vec3(rand() + 50, 0, 0));
		}


		//meshComponent = Entity::Create("Nano suit Entity")->AddComponent<MeshComponent>();
		//model = FileSystem::FileSystemManager::Instance()->LoadObject<Model>("./data/models/nano/nanosuit.fbx");
		//meshComponent->SetModel(model);
		//meshComponent->GetEntity().lock()->GetComponent<TransformComponent>()->SetPosition(glm::vec3(5,0,0));

		auto meshComponent = Entity::Create("Building Entity")->AddComponent<MeshComponent>();
		auto model = FileSystem::FileSystemManager::Instance()->LoadObject<Model>("./data/models/sponza/sponza.obj");
		meshComponent->SetModel(model);

	}

	virtual void Update(const float deltaTime) override
	{
	}

	virtual void Draw() override
	{

	}

	virtual void OnFrameEnd() override
	{
	}

	~EditorApp()
	{
	}
};

Application* CreateApplication()
{
	return new EditorApp();
}