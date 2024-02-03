#include "Graphics/RenderGraphV2/RenderGraphTask.h"

namespace Insight
{
    namespace Graphics
    {
        RenderGraphTask::RenderGraphTask(GPUQueue gpuQueue)
            : GpuQueue(gpuQueue)
        {
        }

        RenderGraphTask::~RenderGraphTask()
        {
        }
    }
}