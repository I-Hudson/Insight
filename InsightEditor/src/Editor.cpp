#include <Insight.h>
#include "Engine/Config/Config.h"
#include "Module/EditorModule.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/string_cast.hpp"
#include "Engine/Graphics/Debug/Gizmos.h"
#include "ImGuizmo.h"

namespace Insight::Editor
{
	class EditorApp : public Application
	{
	public:
		Module::EditorModule* m_editorModule;

		EditorApp() : Application()
		{ }

		virtual void Create() override
		{
#ifdef IMGUI_ENABLED
			m_editorModule = m_moduleManager->AddModule<Insight::Module::EditorModule>();
			m_editorModule->OnCreate();
			Insight::Config::GetInstance().Parse("./data/config/editorConfig.txt");

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

			Entity& mainCamera = Scene::ActiveScene()->CreateEntity("Main Camera");
			CameraComponent& camCom = mainCamera.AddComponent<CameraComponent>();
			camCom.SetProjMatrix(60.0f, CameraAspect::CurrentWindowSize, 0.1f, 4000.0f);
			camCom.SetCameraSpeed(25);

			Entity& dirLight = Scene::ActiveScene()->CreateEntity("DirectionalLight");
			dirLight.GetComponent<TransformComponent>().SetPosition({ 25, 25, 25 });
			DirectionalLightComponent& lightCom = dirLight.AddComponent<DirectionalLightComponent>();
			DirectionalLightComponentData& lightData = lightCom.GetComponentData<DirectionalLightComponentData>();
			lightData.FOV = 15.0f;
			lightData.Direction = glm::normalize(glm::vec3(0, 0, 0) - dirLight.GetComponent<TransformComponent>().GetPostion());
			lightData.NearPlane = 1.0f;
			lightData.FarPlane = 128.0f;

			{
				IS_PROFILE_SCOPE("Loading models - Single Thread");
				for (size_t i = 0; i < 1; ++i)
				{
					//AssetPtr<Model> graphicsModel = Module::AssetModule::Instance()->Load<Model>("./data/models/vulkanscene_shadow.gltf");
					//AssetPtr<Model> graphicsModel = Module::AssetModule::Instance()->Load<Model>("./data/models/Test/testCube.fbx");
					//AssetPtr<Model> graphicsModel = Module::AssetModule::Instance()->Load<Model>("./data/models/Survival_BackPack_2/backpack.obj");
					//AssetPtr<Model> graphicsModel = Module::AssetModule::Instance()->Load<Model>("./data/models/sponza/sponza.obj");
					//Insight::Model* graphicsModel = Insight::FileSystem::FileSystemManager::Instance()->LoadObject<Insight::Model>("./data/models/sponza/sponza.obj");
					AssetPtr<Model> graphicsModel = Module::AssetModule::Instance()->Load<Model>("./data/models/dancing_stormtrooper_gltf/scene.gltf");
					//AssetPtr<Model> graphicsModel = Module::AssetModule::Instance()->Load<Model>("./data/models/vampire/dancing_vampire.dae");

					Entity& animatedMesh = Scene::ActiveScene()->CreateEntity("AnimatedMesh");
					SkinnedMeshComponent& skinnedMesh = animatedMesh.AddComponent<SkinnedMeshComponent>();
					skinnedMesh.SetModel(&*graphicsModel);
					AnimatorComponent& animCom = animatedMesh.AddComponent<AnimatorComponent>();
					animCom.SetSkelton(&graphicsModel->GetMesh().GetSkeleton());
					animCom.PlayAnimation(graphicsModel->GetMesh().GetAnimation(0));
					//graphicsModel->GetMesh().GetAnimation(0)->SetPlayBackSpeed(500);

					Entity& staticMesh = Scene::ActiveScene()->CreateEntity("StaticMesh");
					MeshComponent& meshComponent = staticMesh.AddComponent<MeshComponent>();
					meshComponent.SetModel(&*graphicsModel);
					meshComponent.GetEntity().GetComponent<TransformComponent>().SetPosition({ 15, 0, 0 });
				}
			}

			::New<Graphics::Debug::Gizmos>();

			m_state = ApplicationState::Running;
		}

		virtual void Update(const float deltaTime) override
		{
			//if (!modelAsset)
			//{
			//	modelAsset = Insight::Module::AssetModule::Instance()->LoadAsync<Insight::Assets::ModelAsset>("./data/models/sponza/sponza.obj");
			//}

			//if (modelAsset->IsValid() && modelAsset->IsReady())
			//{
			//	auto modelPtr = modelAsset->GetResult().GetResult();
			//}
		}

		virtual void Draw() override
		{ }

		virtual void OnFrameEnd() override
		{ }

		~EditorApp()
		{
			if (Insight::Graphics::Debug::Gizmos::IsInitialised())
			{
				::Delete(Insight::Graphics::Debug::Gizmos::Instance());
			}
		}
	};

	//std::vector<std::string> modelPaths =
	//{
	//	"./data/models/sponza/sponza.obj",
	//	"./data/models/sponza/sponza.obj",
	//	"./data/models/sponza/sponza.obj"
	//};
	//std::vector<Insight::Model*> loadedModels;
	//loadedModels.resize(modelPaths.size());
	//
	//{
	//	IS_PROFILE_START_CAPTURE();
	//	u32 vecIndex = 0;
	//	for (auto& path : modelPaths)
	//	{
	//		loadedModels[vecIndex] = new Insight::Model(path);
	//		++vecIndex;
	//	}
	//	IS_PROFILE_STOP_CAPTURE();
	//	IS_PROFILE_SAVE_CAPTURE("ModelLoading - SingleThread");
	//}
	//
	//js::JobSystemManager jsManager;
	//js::JobSystemManager::ReturnCode code = jsManager.Init();
	//
	//loadedModels.clear();
	//{
	//	IS_PROFILE_START_CAPTURE();
	//	std::vector<std::shared_ptr<js::JobWithResult<Insight::Model*>>> jsLoadedModels;
	//	js::JobWaitList jsWaitList;
	//	for (size_t i = 0; i < 3; ++i)
	//	{
	//		std::string modelPath = modelPaths[i];
	//		auto job = jsManager.CreateJob(js::JobPriority::Normal, [modelPath]() -> Insight::Model*
	//		{
	//			return new Insight::Model(modelPath);
	//		});
	//		jsLoadedModels.push_back(job);
	//		jsManager.ScheduleJob(job);
	//		jsWaitList.AddJobToWaitOn(job);
	//	}
	//	jsWaitList.Wait();
	//	IS_PROFILE_STOP_CAPTURE();
	//	IS_PROFILE_SAVE_CAPTURE("ModelLoading - MultiThread");
	//}
	//jsManager.Shutdown(true);

}

Insight::Application* CreateApplication()
{
	return new Insight::Editor::EditorApp();
}