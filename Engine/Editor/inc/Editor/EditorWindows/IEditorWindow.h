#pragma once

#include "Core/TypeAlias.h"
#include "Editor/Defines.h"

#include <Reflect/Reflect.h>

#include <string>

namespace Insight
{
	namespace Editor
	{
		enum class EditorWindowCategories
		{
			File,
			Edit,
			Windows
		};

		/// <summary>
		/// Base class for all editor windows.
		/// </summary>
		class IS_EDITOR IEditorWindow
		{
		public:
			IEditorWindow();
			IEditorWindow(u32 minWidth, u32 minHeight);
			IEditorWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight);
			virtual ~IEditorWindow();

			void Draw();

			void SetFullscreen(bool value);
			bool GetFullscreen() const;

			void Show();
			void Hide();

			/// @brief Called once when the window is created on start up.
			virtual void Initialise() { }
			/// @brief Call once when the window is destroyed at editor termination.
			virtual void Shutdown() { }

			virtual void OnDraw() = 0;

			virtual const char* GetWindowName() const = 0;
			virtual EditorWindowCategories GetCategory() const = 0;

			bool IsCursorWithinWindow() const;

		protected:
			bool m_isOpen = true;
			bool m_isFullscreen = false;

			u64 m_imguiWindowFlags = 0;

			u32 m_minWidth = 0;
			u32 m_minHeight = 0;
			u32 m_maxWidth = 0;
			u32 m_maxHeight = 0;

			u32 m_positionX = 0;
			u32 m_positionY = 0;
			u32 m_width = 0;
			u32 m_height = 0;
		};
#define EDITOR_WINDOW(Name, WindowCategory)													\
	static constexpr const char* WINDOW_NAME = #Name;										\
	virtual const char* GetWindowName() const override { return WINDOW_NAME; }				\
	static constexpr ::Insight::Editor::EditorWindowCategories WINDOW_CATEGORY = WindowCategory;				\
	virtual ::Insight::Editor::EditorWindowCategories GetCategory() const override { return WindowCategory; }
	}
}