#pragma once

#include "Core/Singleton.h"
#include "Core/Delegate.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace Insight
{
	namespace Editor
	{
		class IEditorWindow;

		class EditorWindowManager : public Core::Singleton<EditorWindowManager>
		{
			using RegisterWindowFunc = Core::Action<IEditorWindow*()>;
		public:
			EditorWindowManager();
			~EditorWindowManager();

			void RegisterWindows();
			void AddWindow(const std::string& windowName);
			void RemoveWindow(const std::string& windowName);

			bool IsWindowVisable(const std::string& windowName) const;

			void Update();
			void Destroy();

		private:
			void RemoveQueuedWindows();

		private:
			std::unordered_map<std::string, RegisterWindowFunc> m_windowRegistry;
			std::vector<IEditorWindow*> m_activeWindows;
			std::vector<std::string> m_windowsToRemove;
		};
	}
}