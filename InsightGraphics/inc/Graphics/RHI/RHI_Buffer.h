#pragma once

#include "Graphics/RHI/RHI_Resource.h"

namespace Insight
{
	namespace Graphics
	{
		class RenderContext;

		class RHI_Buffer : public RHI_Resource
		{
		public:

			static RHI_Buffer* New();

		private:
			friend class RenderContext;
		};
	}
}