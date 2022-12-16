#pragma once

namespace Insight
{
	namespace Core
	{
		class ImGuiSystem;
	}

	class EditorModule
	{
	public:
		static void Initialise(Core::ImGuiSystem* imguiSystem);
	};
}