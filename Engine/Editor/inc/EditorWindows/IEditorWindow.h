#pragma once

#include "Core/TypeAlias.h"

#include <string>

namespace Insight
{
	namespace Editor
	{
		/// <summary>
		/// Base class for all editor windows.
		/// </summary>
		class IEditorWindow
		{
		public:
			IEditorWindow();
			IEditorWindow(u32 minWidth, u32 minHeight);
			IEditorWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight);
			~IEditorWindow();

			void Draw();

			virtual void OnDraw() = 0;
			virtual const char* GetWindowName() const = 0;

		protected:
			bool m_isOpen = true;

			u32 m_minWidth = 0;
			u32 m_minHeight = 0;
			u32 m_maxWidth = 0;
			u32 m_maxHeight = 0;

			u32 m_width = 0;
			u32 m_height = 0;
		};
#define EDITOR_WINDOW(Name) \
	static constexpr char* WINDOW_NAME = #Name; \
	virtual const char* GetWindowName() const override { return WINDOW_NAME; }
	}
}