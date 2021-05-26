#pragma once

#include "Engine/Core/Core.h"
#include "Editor/EditorWindow.h"
#include "Engine/Entity/Entity.h"
#include "Engine/Entity/EntityManager.h"
#include "Engine/Entity/ComponentManager.h"

#ifdef IS_EDITOR
namespace Insight::Editor
{
	class SceneWindow : public EditorWindow
	{
	public:
		SceneWindow(const Module::EditorModule* editorModule);
		virtual ~SceneWindow() override;

		virtual void Update(const float& deltaTime) override;

		const glm::ivec2& GetWindowSize() const { return m_windowSize; }
	private:
		void CheckForWindowResize();

	private:
		EntityManager m_entityManager;
		ComponentManager m_componentManager;
		Entity m_editorCamera;

		glm::ivec2 m_windowSize;
	};
}
#endif