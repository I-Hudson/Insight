#include <Insight.h>
#include <../vendor/glm/glm/gtx/string_cast.hpp>

using namespace Insight::Maths;

class Sandbox : public Insight::Application
{
public:
	Entity* e;
	TransformComponent* tc;
	Model* testModel;

	Sandbox() : Insight::Application()
	{
		testModel = Insight::Memory::MemoryManager::NewOnFreeList<Model>("./models/Survival_BackPack_2/Survival_BackPack_2.fbx");

		e = Insight::Memory::MemoryManager::NewOnFreeList<Entity>();
		tc = e->AddComponent<TransformComponent>();
		e->AddComponent<MeshComponent>();
		e->GetComponent<MeshComponent>()->SetMesh(testModel->GetSubMesh(0));

		for (size_t i = 1; i < testModel->GetSubMeshCount(); i++)
		{
			e->AddChild(Insight::Memory::MemoryManager::NewOnFreeList<Entity>(std::to_string(i)));
			auto mesh = e->GetChild(i - 1)->AddComponent<MeshComponent>();
			mesh->SetMesh(testModel->GetSubMesh(i));
		}	
	}

	virtual void Update(const float deltaTime) override
	{
		double x, y;
		Input::GetMousePosition(&x, &y);

		if (Input::KeyHeld(KEY_W))
		{
			tc->SetPosition(tc->GetPostion() + glm::vec3(0, 0, 1));
		}
		if (Input::KeyHeld(KEY_S))
		{
			tc->SetPosition(tc->GetPostion() + glm::vec3(0, 0, -1));
		}
		if (Input::KeyHeld(KEY_A))
		{
			tc->SetPosition(tc->GetPostion() + glm::vec3(-1, 0, 1));
		}
		if (Input::KeyHeld(KEY_D))
		{
			tc->SetPosition(tc->GetPostion() + glm::vec3(1, 0, 1));
		}


		//IS_CORE_INFO("MouseX: {0}, MouseY: {1}", x, y);
		//IS_CORE_INFO("{0}", 1.0f / deltaTime);
		glm::vec3 p = e->GetComponent<TransformComponent>()->GetPostion();
		IS_CORE_INFO("Transform {0}", glm::to_string(p));
	}

	virtual void Draw() override
	{

	}

	~Sandbox()
	{
		Insight::Memory::MemoryManager::DeleteOnFreeList(testModel);
	}
};

Insight::Application* Insight::CreateApplication()
{
	return new Sandbox();
}