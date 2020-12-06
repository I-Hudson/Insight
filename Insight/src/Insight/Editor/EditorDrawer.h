#pragma once

#include "Insight/Core.h"
#include "Insight/Memory/MemoryManager.h"
#include "Insight/Editor/UIHelper.h"

#if defined(IS_EDITOR)
namespace Insight
{
	class Object;

	namespace Editor
	{
		struct EditorDrawerRegistry;

		class IS_API EditorDrawer
		{
		public:
			virtual void OnDraw(Object& obj) { }
		};

		struct EditorDrawerRegistry : TSingleton<EditorDrawerRegistry>
		{
		public:
			EditorDrawerRegistry()
			{
				SetInstancePtr(this);
			}

			~EditorDrawerRegistry()
			{
				Destroy();
				ClearPtr();
			}

			template<typename EditorDrawerClass, typename ObjectClass>
			void AddEditorDrawer()
			{
				IS_CORE_STATIC_ASSERT(EditorDrawer, EditorDrawerClass, "'EditorDrawerClass' does not inherit 'EditorDrawer'.");

				std::string objectClassName = GET_SHORT_NAME_OF_TYPE(ObjectClass);
				if (m_editorDrawers.find(objectClassName) == std::end(m_editorDrawers))
				{
					EditorDrawerClass* tPtr = NEW_ON_HEAP(EditorDrawerClass);
					m_editorDrawers[objectClassName] = tPtr;
				}
			}

			bool CallEditorDrawer(const std::string& typeName, Object& obj)
			{
				auto& it = m_editorDrawers.find(typeName);
				if (it != std::end(m_editorDrawers))
				{
					(*it).second->OnDraw(obj);
					return true;
				}
				return false;
			}

			void Destroy()
			{
				for (auto& drawer : m_editorDrawers)
				{
					DELETE_ON_HEAP(drawer.second);
				}
			}

		private:
			std::unordered_map<std::string, EditorDrawer*> m_editorDrawers;
		};
	}
}
#define CUSTOM_EDITOR_DRAWER(editorDrawer, objectClass) Insight::Editor::EditorDrawerRegistry::Instance()->AddEditorDrawer<editorDrawer, objectClass>()
#else
#define CUSTOM_EDITOR_DRAWER(editorDrawer, objectClass)
#endif
