#pragma once

#include "Runtime/Engine.h"

namespace Insight
{
	namespace Standalone
	{
		class StandaloneApp : public Insight::App::Engine
		{
		public:

			virtual void OnInit() override;
			virtual void OnUpdate() override;
			virtual void OnDestroy() override;

		private:
		};
	}
}