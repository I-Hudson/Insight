#pragma once

namespace Insight
{
	namespace Core
	{
		class ImGuiSystem;
	}

	class StandaloneModule
	{
	public:
		static void Initialise(Core::ImGuiSystem* imguiSystem);
	};
}