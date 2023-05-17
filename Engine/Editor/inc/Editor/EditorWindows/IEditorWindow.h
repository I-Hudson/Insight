#pragma once

#include "Core/TypeAlias.h"

#include <Reflect.h>

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
		class IEditorWindow
		{
		public:
			IEditorWindow();
			IEditorWindow(u32 minWidth, u32 minHeight);
			IEditorWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight);
			virtual ~IEditorWindow();

			void Draw();

			void SetFullscreen(bool value);
			bool GetFullscreen() const;

			virtual void Initialise() { }
			virtual void Shutdown() { }

			virtual void OnDraw() = 0;

			virtual const char* GetWindowName() const = 0;
			virtual EditorWindowCategories GetCategory() const = 0;

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
	static constexpr const char* WINDOW_NAME = #Name;												\
	virtual const char* GetWindowName() const override { return WINDOW_NAME; }				\
	static constexpr EditorWindowCategories WINDOW_CATEGORY = WindowCategory;				\
	virtual EditorWindowCategories GetCategory() const override { return WindowCategory; }
	}
}