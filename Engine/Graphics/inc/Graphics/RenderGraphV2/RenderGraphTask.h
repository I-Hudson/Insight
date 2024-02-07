#pragma once

#ifdef RENDERGRAPH_V2_ENABLED

#include "Graphics/Defines.h"
#include "Graphics/Enums.h"
#include "Graphics/PipelineBarrier.h"

#include <vector>

namespace Insight
{
    namespace Graphics
    {
        using RGTextureHandle = int;
        using RGBufferHandle = int;

        class RHI_CommandList;
        class RenderGraphV2;

        /// @brief Define a single task/job to be execute within the render graph. This could be a graphics pass, to a buffer/texture upload/copy etc.
        class IS_GRAPHICS RenderGraphTask
        {
        public:
            RenderGraphTask() = delete;
            RenderGraphTask(RenderGraphV2* renderGraph, GPUQueue gpuQueue);
            virtual ~RenderGraphTask();

            void SetCommandList(RHI_CommandList* cmdList);

            virtual const char* GetTaskName() const = 0;

            /// @brief Called before any Execute function is invoked.
            virtual void PreExecute() = 0;
            virtual void Execute() = 0;
            /// @brief Called after all other Execute functions have been invoked.
            virtual void PostExecute() = 0;
        
            std::vector<PipelineBarrier> PipelineBarriers;
            RenderGraphV2* RenderGraph = nullptr;
            RHI_CommandList* CmdList = nullptr;
            GPUQueue GpuQueue;
        };
    }
}
#endif