#pragma once

#include "Runtime/Engine.h"

#include "EditorWindows/EditorWindowManager.h"

namespace Insight
{
	namespace Editor
	{
		class Editor : public Insight::App::Engine
		{
		public:

			virtual void OnInit() override;
			virtual void OnUpdate() override;
			virtual void OnDestroy() override;

		private:
			EditorWindowManager m_editorWindowManager;
		};
	}
}