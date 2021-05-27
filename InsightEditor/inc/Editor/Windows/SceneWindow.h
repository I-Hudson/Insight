#pragma once

#include "Engine/Core/Core.h"
#include "Editor/EditorWindow.h"
#include "Engine/Entity/Entity.h"
#include "Engine/Entity/EntityManager.h"
#include "Engine/Entity/ComponentManager.h"
#include "Engine/Graphics/GPUDescriptorSet.h"

#ifdef IS_EDITOR
namespace Insight::Editor
{
	struct SceneWindowCleanImage
	{
		std::vector<void*> Descriptors;
		u64 FrameTriggered;
	};

	class SceneWindow : public EditorWindow
	{
	public:
		SceneWindow(const Module::EditorModule* editorModule);
		virtual ~SceneWindow() override;

		virtual void Update(const float& deltaTime) override;

		const glm::ivec2& GetWindowSize() const { return m_windowSize; }
	private:
		void CheckForWindowResize();
		void CheckForImageClean();

	private:
		EntityManager m_entityManager;
		ComponentManager m_componentManager;
		Entity m_editorCamera;

		std::vector<void*> m_sceneImages;
		std::queue<SceneWindowCleanImage> m_imagesToClean;

		u32 m_backBufferSource;
		bool m_windowedHovered;
		glm::ivec2 m_windowSize;
	};
}
#endif