#pragma once

#include "Insight/Core/Core.h"
#include "Insight/Memory/MemoryManager.h"
#include "Editor/UIHelper.h"

#if defined(IS_EDITOR)

#define EDITOR_DEC_TYPE(TYPE, EDITOR_TYPE) \
	static Insight::Editor::EditorDrawerRegister<TYPE, EDITOR_TYPE> s_reg;
#define EDITOR_DEF_TYPE(TYPE, EDITOR_TYPE) \
	Insight::Editor::EditorDrawerRegister<TYPE, EDITOR_TYPE> EDITOR_TYPE::s_reg

namespace Insight
{
	class Object;

	namespace Editor
	{
		class EditorDrawerRegistry;

		class IS_API IEditorDrawer
		{
		public:
			virtual void OnDraw(Object& obj) = 0;

		private:

			Type m_objectType;

			friend EditorDrawerRegistry;
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

		class IS_API EditorDrawerRegistry
		{
		public:
			~EditorDrawerRegistry()
			{
				Destroy();
			}

			template<typename TObjectClass , typename TEditorDrawerClass>
			static void AddEditorDrawer()
			{
				IS_CORE_STATIC_ASSERT((std::is_base_of<IEditorDrawer, TEditorDrawerClass>::value), "'EditorDrawerClass' does not inherit 'EditorDrawer'.");

				Type type = Type();
				type.SetType<TObjectClass>();

				auto it = GetTypes().find(type);
				if (it == GetTypes().end())
				{
					SharedPtr<TEditorDrawerClass> tPtr = CreateSharedPtr<TEditorDrawerClass>();
					SharedPtr<IEditorDrawer> iEdtiroDrawerPtr = StaticPointerCast<IEditorDrawer>(tPtr);
					iEdtiroDrawerPtr->m_objectType = type;

					GetTypes().insert(std::pair(iEdtiroDrawerPtr->m_objectType, tPtr));
				}
			}

			static bool CallEditorDrawer(const Type& typeName, Object& obj)
			{
				auto it = GetTypes().find(typeName);
				auto f = GetTypes();
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

			typedef std::unordered_map<Type, SharedPtr<IEditorDrawer>> CustomEditorDrawerTypes;

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
