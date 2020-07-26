#include <Insight.h>
#include <../vendor/glm/glm/gtx/string_cast.hpp>

using namespace Insight::Maths;

class Sandbox : public Insight::Application
{
public:
	Entity* e = nullptr;
	TransformComponent* tc = nullptr;
	Model* testModel = nullptr;

	Sandbox() : Insight::Application()
	{
		testModel = NEW_ON_HEAP(Model, "./models/nano/nanosuit.fbx");
		
		e = NEW_ON_HEAP(Entity);
		//tc = e->AddComponent<TransformComponent>();
		//e->AddComponent<MeshComponent>();
		//e->GetComponent<MeshComponent>()->SetMesh(testModel->GetSubMesh(0));
		//
		//for (UINT i = 1; i < testModel->GetSubMeshCount(); i++)
		//{
		//	e->AddChild(Insight::Memory::MemoryManager::NewOnFreeList<Entity>(std::to_string(i)));
		//	auto mesh = e->GetChild(i - 1)->AddComponent<MeshComponent>();
		//	mesh->SetMesh(testModel->GetSubMesh(i));
		//}	
	}

	virtual void Update(const float deltaTime) override
	{
		double x, y;
		Input::GetMousePosition(&x, &y);

		if (Input::KeyHeld(KEY_UP))
		{
			tc->SetPosition(tc->GetPostion() + glm::vec3(0, 0, 1));
		}
		if (Input::KeyHeld(KEY_DOWN))
		{
			tc->SetPosition(tc->GetPostion() + glm::vec3(0, 0, -1));
		}
		if (Input::KeyHeld(KEY_LEFT))
		{
			tc->SetPosition(tc->GetPostion() + glm::vec3(-1, 0, 1));
		}
		if (Input::KeyHeld(KEY_RIGHT))
		{
			tc->SetPosition(tc->GetPostion() + glm::vec3(1, 0, 1));
		}
	}

	virtual void Draw() override
	{

	}

	~Sandbox()
	{
		DELETE_ON_HEAP(testModel);
		//DELETE_ON_HEAP(e)
	}
};

Insight::Application* Insight::CreateApplication()
{
	return new Sandbox();
}