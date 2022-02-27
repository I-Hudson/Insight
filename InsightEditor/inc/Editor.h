#pragma once

#include "Core/Engine.h"

namespace Insight
{
	namespace Editor
	{
		class Editor : public Insight::Core::Engine
		{
		public:

			virtual void OnInit() override;
			virtual void OnDestroy() override;

		private:

		};
	}
}