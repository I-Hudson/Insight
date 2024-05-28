#pragma once

namespace Insight
{
	namespace Core
	{
		class ImGuiSystem;
	}

	class RuntimeModule
	{
	public:
		static void Initialise(Core::ImGuiSystem* imguiSystem);
	};
}