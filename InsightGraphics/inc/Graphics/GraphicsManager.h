#pragma once

#include "Defines.h"
#include "Core/ManagerInterface.h"

namespace Insight
{
	namespace Graphics
	{
		class IS_GRAPHICS GraphicsManager : public Insight::Core::ManagerInterface
		{
		public:

			virtual bool Init() override;
			virtual void Update(const float deltaTime) override;
			virtual void Destroy() override;

		private:
		};
	}
}