#pragma once

#include "Core/Singleton.h"
#include "Core/Delegate.h"

#include "Editor/EditorWindows/IEditorWindow.h"

#include "Core/Memory.h"
#include "Serialisation/Serialiser.h"

#include "Algorithm/Vector.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace Insight
{
	namespace Editor
	{
		class IEditorWindow;

		struct IS_EDITOR RegisterWindow
		{
			using RegisterWindowFunc = Core::Action<IEditorWindow*()>;

			RegisterWindow() = default;

			template<typename Lambda>
			RegisterWindow(Lambda func, EditorWindowCategories category)
			{
				RegisterFunc.Bind(func);
				Category = category;
			}

			RegisterWindowFunc RegisterFunc;
			EditorWindowCategories Category;
		};

		class IS_EDITOR EditorWindowManager : public Core::Singleton<EditorWindowManager>, public Serialisation::ISerialisable
		{
		public:
			IS_SERIALISABLE_H(EditorWindowManager);

			EditorWindowManager();
			~EditorWindowManager();

			void RegisterWindows();

			template<typename T>
			void RegisterEditorWindow()
			{
				if (m_windows.find(T::WINDOW_NAME) != m_windows.end())
				{
					return;
				}
				m_windows[T::WINDOW_NAME] = static_cast<IEditorWindow*>(New<T, Core::MemoryAllocCategory::Editor>());
				m_windows[T::WINDOW_NAME]->Initialise();
			}

			template<typename T>
			void UnregisterEditorWindow()
			{
				static_assert(std::is_base_of_v<IEditorWindow, T>);
				RemoveWindowNow(T::WINDOW_NAME); 
				
				if (auto iter = m_windows.find(T::WINDOW_NAME); 
					iter != m_windows.end())
				{
					IEditorWindow* window = iter->second;
					window->Shutdown();
					Delete(window);
					m_windows.erase(iter);
				}
			}

			void AddWindow(const std::string& windowName);
			void RemoveWindow(const std::string& windowName);
			/// @brief Remove a window straight away, should be used with caution.
			void RemoveWindowNow(std::string_view windowName);

			bool IsWindowVisable(const std::string& windowName) const;

			std::vector<std::string> GetAllRegisteredWindowNames(EditorWindowCategories category) const;
			std::vector<std::string> GetAllActiveWindowNames(EditorWindowCategories category) const;
			std::vector<IEditorWindow const*> GetAllActiveWindows(EditorWindowCategories category) const;

			std::vector<std::string> GetAllRegisteredWindowNames() const;
			std::vector<std::string> GetAllActiveWindowNames() const;
			IEditorWindow* GetActiveWindow(std::string_view windowName);
			void RemoveAllWindows();

			void Update();
			void Destroy();

		private:
			void RemoveQueuedWindows();

		private:
			std::unordered_map<std::string, IEditorWindow*> m_windows;
			std::vector<IEditorWindow*> m_activeWindows;
			std::vector<std::string> m_windowsToRemove;
		};
	}

	namespace Serialisation
	{
		struct EditorWindowManager1 { };
		template<>
		struct ComplexSerialiser<EditorWindowManager1, std::vector<Editor::IEditorWindow*>, Editor::EditorWindowManager>
		{
			void operator()(ISerialiser* serialiser, std::vector<Editor::IEditorWindow*>& activeWindows, Editor::EditorWindowManager* windowManager) const
			{
				constexpr const char* c_ActiveWindows = "ActiveWindows";
				if (serialiser->IsReadMode())
				{
					u64 arraySize = 0;
					serialiser->StartArray(c_ActiveWindows, arraySize);
					for (u32 i = 0; i < arraySize; ++i)
					{
						std::string windowName;
						serialiser->Read("", windowName);

						windowManager->AddWindow(windowName);
					}
					serialiser->StopArray();
				}
				else
				{
					u64 arraySize = activeWindows.size();
					serialiser->StartArray(c_ActiveWindows, arraySize);
					for (const Editor::IEditorWindow* window : activeWindows)
					{
						serialiser->Write("", window->GetWindowName());
					}
					serialiser->StopArray();
				}
			}
		};
	}

	OBJECT_SERIALISER(Editor::EditorWindowManager, 1,
		SERIALISE_COMPLEX(Serialisation::EditorWindowManager1, m_activeWindows, 1, 0)
	)
}