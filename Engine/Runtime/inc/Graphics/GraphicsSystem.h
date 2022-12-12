#pragma once

#include "Core/ISysytem.h"
#include "Core/Singleton.h"
#include "Graphics/RenderContext.h"
#include "Graphics/Window.h"
#include "Graphics/RenderGraph/RenderGraph.h"

namespace Insight
{
	namespace Runtime
	{
		/// <summary>
		/// System in charge of all things graphics.
		/// This system should init a graphics device/context and window ready for rendering.
		/// </summary>
		class GraphicsSystem : public Core::Singleton<GraphicsSystem>, public Core::ISystem
		{
		public:
			GraphicsSystem();
			virtual ~GraphicsSystem() override;

			IS_SYSTEM(GraphicsSystem);

			virtual void Initialise() override;
			virtual void Shutdown() override;

			void Update();

		private:
			Graphics::RenderContext* m_context = nullptr;
			Graphics::Window m_window;
			Graphics::RenderGraph m_renderGraph;
		};
	}
}