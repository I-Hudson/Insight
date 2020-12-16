#pragma once

#include "Insight/Core.h"
#include "Insight/Templates/TSingleton.h"

namespace Insight
{
	class Renderer;

	class ImGuiRenderer : public Insight::TSingleton<ImGuiRenderer>
	{
	public:
		ImGuiRenderer();
		virtual ~ImGuiRenderer();

		virtual void Init(SharedPtr<Renderer> renderer) = 0;
		virtual void NewFrame() = 0;
		virtual void EndFrame() = 0;

		static SharedPtr<ImGuiRenderer> Create();
	};
}