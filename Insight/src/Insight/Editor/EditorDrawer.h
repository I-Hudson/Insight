#pragma once

#include "Insight/Core/Core.h"
#include "Insight/Memory/MemoryManager.h"
#include "Insight/Editor/UIHelper.h"

#if defined(IS_EDITOR)
namespace Insight
{
	class Object;

	namespace Editor
	{
		class IS_API IEditorDrawer
		{
		public:
			virtual void OnDraw(SharedPtr<Object> obj) = 0;
		};

		template<typename TObjectClass, typename TEditorDrawerClass>
		struct IS_API EditorDrawerRegister
		{
			EditorDrawerRegister()
			{
				IS_CORE_STATIC_ASSERT((std::is_base_of<IEditorDrawer, TEditorDrawerClass>::value), "'TEditorDrawerClass' does not inherit from 'IEditorDrawer'.");
				EditorDrawerRegistry::AddEditorDrawer<TObjectClass, TEditorDrawerClass>();
			}
		};

		template<typename TObjectClass, typename TEditorDrawerClass>
		class IS_API TEditorDrawer : public IEditorDrawer
		{
		private:
			static EditorDrawerRegister<TObjectClass, TEditorDrawerClass> s_editorDrawerRegister;
		};


		class IS_API EditorDrawerRegistry
		{
		public:
			EditorDrawerRegistry()
			{
			}

			~EditorDrawerRegistry()
			{
				Destroy();
			}

			template<typename TObjectClass , typename TEditorDrawerClass>
			static void AddEditorDrawer()
			{
				IS_CORE_STATIC_ASSERT((std::is_base_of<IEditorDrawer, TEditorDrawerClass>::value), "'EditorDrawerClass' does not inherit 'EditorDrawer'.");

				std::string objectClassName = typeid(ObjectClass).name();
				if (!m_editorDrawers.at(objectClassName))
				{
					SharedPtr<TEditorDrawerClass> tPtr = CreateSharedPtr<TEditorDrawerClass>();
					GetTypes().insert(std::pair(objectClassName, tPtr));
				}
			}

			static bool CallEditorDrawer(const std::string& typeName, SharedPtr<Object> obj)
			{
				auto& it = GetTypes().find(typeName);
				if (it != GetTypes().end())
				{
					(*it).second->OnDraw(obj);
					return true;
				}
				return false;
			}

			static void Destroy()
			{
				for (auto& drawer : GetTypes())
				{
					drawer.second.reset();
				}
			}

			typedef std::unordered_map<std::string, SharedPtr<IEditorDrawer>> CustomEditorDrawerTypes;

			static CustomEditorDrawerTypes& GetTypes()
			{
				static CustomEditorDrawerTypes types;
				return types;
			}
		};
	}
}
#else
#define REGISTER_EDITOR_DRAWER_TYPE(DataClass, EditorClass)
#endif
