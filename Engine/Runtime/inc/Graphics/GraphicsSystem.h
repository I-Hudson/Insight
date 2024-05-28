#pragma once

#include "Core/ISysytem.h"
#include "Core/Singleton.h"
#include "Core/Collections/DoubleBufferVector.h"
#include "Graphics/RenderContext.h"
#include "Graphics/Window.h"
#include "Graphics/RenderFrame.h"

namespace Insight
{
	namespace Input
	{
		class InputSystem;
	};

	namespace Runtime
	{
		/// <summary>
		/// System in charge of all things graphics.
		/// This system should init a graphics device/context and window ready for rendering.
		/// </summary>
		class IS_RUNTIME GraphicsSystem : public Core::ISystem
		{
		public:
			GraphicsSystem();
			virtual ~GraphicsSystem() override;

			IS_SYSTEM(GraphicsSystem);

			void Initialise(Input::InputSystem* inputSystem);
			virtual void Shutdown() override;

			void Update();
			void Render();

			void CreateRenderFrame();
			const RenderFrame& GetRenderFrame() const;

		private:
			void InitialiseRenderContext(Graphics::GraphicsAPI graphicsAPI);

		private:
			Graphics::RenderContext* m_context = nullptr;
			Graphics::Window m_window;

			DoubleBufferVector<RenderFrame> m_renderFrame;
			Input::InputSystem* m_inputSystem = nullptr;
		};
	}
}