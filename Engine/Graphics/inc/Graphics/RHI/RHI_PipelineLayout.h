#pragma once

#include "Graphics/RHI/RHI_Resource.h"
#include "Graphics/PipelineStateObject.h"

namespace Insight
{
    namespace Graphics
    {
        class RenderContext;

        class RHI_PipelineLayout : public RHI_Resource
        {
        public:
            virtual ~RHI_PipelineLayout() { }

            static RHI_PipelineLayout* New();
            virtual void Create(RenderContext* context, PipelineStateObject pso) = 0;
        };
    }
}