#pragma once

#include "Graphics/Enums.h"
#include "Graphics/PipelineBarrier.h"

#include <vector>

namespace Insight
{
    namespace Graphics
    {
        using RGTextureHandle = int;
        using RGBufferHandle = int;

        /// @brief Define a single task/job to be execute within the render graph. This could be a graphics pass, to a buffer/texture upload/copy etc.
        class RenderGraphTask
        {
        public:
            RenderGraphTask() = delete;
            RenderGraphTask(GPUQueue gpuQueue);
            virtual ~RenderGraphTask();

            virtual const char* GetTaskName() const = 0;

            std::vector<RGBufferHandle> BufferReads;
            std::vector<RGBufferHandle> BufferWrites;

            std::vector<RGTextureHandle> TextureReads;
            std::vector<RGTextureHandle> TextureWrites;
        
            std::vector<PipelineBarrier> PipelineBarriers;

            GPUQueue GpuQueue;
        };
    }
}