#ifdef RENDERGRAPH_V2_ENABLED

#include "Graphics/RenderGraphV2/RenderGraphTask.h"
#include "Platforms/Platform.h"

namespace Insight
{
    namespace Graphics
    {
        RenderGraphTask::RenderGraphTask(RenderGraphV2* renderGraph, GPUQueue gpuQueue)
            : RenderGraph(renderGraph)
            , GpuQueue(gpuQueue)
        {
        }

        RenderGraphTask::~RenderGraphTask()
        {
        }

        void RenderGraphTask::SetCommandList(RHI_CommandList* cmdList)
        {
            ASSERT_MSG(CmdList == nullptr, "CommandList should be nullptr and not reassigned.");
            CmdList = cmdList;
        }
    }
}
#endif