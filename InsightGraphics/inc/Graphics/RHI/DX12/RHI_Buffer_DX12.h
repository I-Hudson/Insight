#pragma once

#include "Graphics/RHI/RHI_Buffer.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			class RHI_Buffer_DX12 : public RHI_Buffer
			{
			public:

				virtual void Release() override { }

			private:

			};
		}
	}
}