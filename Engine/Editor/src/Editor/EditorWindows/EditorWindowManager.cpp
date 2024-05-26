#include "Editor/EditorWindows/EditorWindowManager.h"
#include "Editor/EditorWindows/IEditorWindow.h"

#include "Core/Profiler.h"

#include "EditorWindows.gen.h"

namespace Insight
{
	namespace Editor
	{
		EditorWindowManager::EditorWindowManager()
		{
		}

		EditorWindowManager::~EditorWindowManager()
		{
			Destroy();
		}

		IS_SERIALISABLE_CPP(EditorWindowManager)

		void EditorWindowManager::RegisterWindows()
		{
			RegisterAllEditorWindows();
		}

		void EditorWindowManager::AddWindow(const std::string& windowName)
		{
			if (IsWindowVisable(windowName))
			{
				return;
			}

			auto itr = m_windows.find(windowName);
			if (itr != m_windows.end())
			{
				IEditorWindow* newWindow = itr->second;
				newWindow->Show();
				m_activeWindows.push_back(newWindow);
			}
		}

		void EditorWindowManager::RemoveWindow(const std::string& windowName)
		{
			m_windowsToRemove.push_back(windowName);
		}

		void EditorWindowManager::RemoveWindowNow(std::string_view windowName)
		{
			if (auto itr = std::find_if(m_activeWindows.begin(), m_activeWindows.end(), [windowName](const IEditorWindow* window)
				{
					return window->GetWindowName() == windowName;
				}); itr != m_activeWindows.end())
			{
				(*itr)->Hide();
				m_activeWindows.erase(itr);
			}
		}

		bool EditorWindowManager::IsWindowVisable(const std::string& windowName) const
		{
			for (size_t i = 0; i < m_activeWindows.size(); ++i)
			{
				if (windowName == m_activeWindows.at(i)->GetWindowName())
				{
					return true;
				}
			}
			return false;
		}

		std::vector<std::string> EditorWindowManager::GetAllRegisteredWindowNames(EditorWindowCategories category) const
		{
			std::vector<std::string> result;
			for (auto& [name, window] : m_windows)
			{
				if (window->GetCategory() == category)
				{
					result.push_back(name);
				}
			}
			return result;
		}

		std::vector<std::string> EditorWindowManager::GetAllActiveWindowNames(EditorWindowCategories category) const
		{
			std::vector<std::string> result;
			for (IEditorWindow const* window : m_activeWindows)
			{
				if (window->GetCategory() == category)
				{
					result.push_back(window->GetWindowName());
				}
			}
			return result;
		}

		std::vector<IEditorWindow const*> EditorWindowManager::GetAllActiveWindows(EditorWindowCategories category) const
		{
			std::vector< IEditorWindow const*> result;
			for (IEditorWindow const* window : m_activeWindows)
			{
				if (window->GetCategory() == category)
				{
					result.push_back(window);
				}
			}
			return result;
		}

		std::vector<std::string> EditorWindowManager::GetAllRegisteredWindowNames() const
		{
			std::vector<std::string> result;
			for (auto& [name, window] : m_windows)
			{
				result.push_back(name);
			}
			return result;
		}

		std::vector<std::string> EditorWindowManager::GetAllActiveWindowNames() const
		{
			std::vector<std::string> result;
			for (IEditorWindow const* window : m_activeWindows)
			{
				result.push_back(window->GetWindowName());
			}
			return result;
		}

		IEditorWindow* EditorWindowManager::GetActiveWindow(std::string_view windowName)
		{
			for (IEditorWindow* window : m_activeWindows)
			{
				if (window->GetWindowName() == windowName)
				{
					return window;
				}
			}
			return nullptr;
		}

		void EditorWindowManager::RemoveAllWindows()
		{
			std::vector<std::string> activeWindows = GetAllActiveWindowNames();
			for (size_t i = 0; i < activeWindows.size(); ++i)
			{
				RemoveWindow(activeWindows.at(i));
			}
		}

		void EditorWindowManager::Update()
		{
			IS_PROFILE_FUNCTION();

			RemoveQueuedWindows();
			for (size_t i = 0; i < m_activeWindows.size(); ++i)
			{
				m_activeWindows.at(i)->Draw();
			}
		}

		void EditorWindowManager::Destroy()
		{
			for (auto& [name, window] : m_windows)
			{
				window->Shutdown();
				Delete(window);
			}
			m_windows.clear();
			m_activeWindows.clear();
			m_windowsToRemove.clear();
		}

		void EditorWindowManager::RemoveQueuedWindows()
		{
			for (size_t i = 0; i < m_windowsToRemove.size(); ++i)
			{
				std::string_view windowToRemove = m_windowsToRemove.at(i);
				if (auto itr = std::find_if(m_activeWindows.begin(), m_activeWindows.end(), [windowToRemove](const IEditorWindow* window)
					{
						return window->GetWindowName() == windowToRemove;
					}); itr != m_activeWindows.end())
				{
					(*itr)->Hide();
					m_activeWindows.erase(itr);
				}
			}
			m_windowsToRemove.clear();
		}
	}
}