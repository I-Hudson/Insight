#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Templates/TSingleton.h"

	class Renderer;

	class ImGuiRenderer : public TSingleton<ImGuiRenderer>
	{
	public:
		ImGuiRenderer();
		virtual ~ImGuiRenderer();

		virtual void Init(SharedPtr<Renderer> renderer) = 0;
		virtual void NewFrame() = 0;
		virtual void EndFrame() = 0;

		static SharedPtr<ImGuiRenderer> Create();
	};