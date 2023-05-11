#pragma once

#include "Runtime/Engine.h"

namespace Insight
{
	namespace Graphics
	{
		class Renderpass;
	}

	namespace Standalone
	{
		class StandaloneApp : public Insight::App::Engine
		{
		public:

			virtual void OnInit() override;
			virtual void OnUpdate() override;
			virtual void OnRender() override;
			virtual void OnDestroy() override;

		private:
			Graphics::Renderpass* m_gameRenderpass = nullptr;
		};
	}
}