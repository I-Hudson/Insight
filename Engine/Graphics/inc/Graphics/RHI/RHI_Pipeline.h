#pragma once

#include "Graphics/RHI/RHI_Resource.h"
#include "Graphics/PipelineStateObject.h"

namespace Insight
{
	namespace Graphics
	{
		class RenderContext;

		class RHI_Pipeline : public RHI_Resource
		{
		public:
			virtual ~RHI_Pipeline() { }

			static RHI_Pipeline* New();
			virtual void Create(RenderContext* context, PipelineStateObject pso) = 0;

			ShaderDesc ShaderDesc;
		};
	}
}