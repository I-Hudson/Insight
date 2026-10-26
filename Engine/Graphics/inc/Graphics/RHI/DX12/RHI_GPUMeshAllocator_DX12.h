#pragma once

#if defined(IS_DX12_ENABLED)

#include "Graphics/RHI/RHI_GPUMeshAllocator.h"

#include "D3D12MemAlloc.h"

#include <unordered_map>

namespace Insight
{
    namespace Graphics
    {
        namespace RHI::DX12
        {
            class RenderContext_DX12;
            class RHI_Buffer_DX12;

            class RHI_GPUMeshAllocator_DX12 : public RHI_GPUMeshAllocator
            {
            public:
                virtual ~RHI_GPUMeshAllocator_DX12() override;

                virtual void Initialise(RenderContext* renderContext) override;
                virtual void Shutdown() override;

                virtual RHI_GPUMeshAllocatorHandle AllocateMeshLOD(void* vertexData, u64 vertexSize, void* indexData, u64 indexSize, GPU_MeshLODMetaData& meshLODMetadata) override;
                virtual void FreeMeshLOD(RHI_GPUMeshAllocatorHandle handle) override;

            private:
                RenderContext_DX12* m_renderContext = nullptr;
                D3D12MA::VirtualBlock* m_meshBufferAllocator;

                RHI_Buffer_DX12* m_gpuMeshBuffer = nullptr;

                std::unordered_map<RHI_GPUMeshAllocatorHandle, D3D12MA::VirtualAllocation> m_allocations;
            };
        }
    }
}
#endif // defined(IS_DX12_ENABLED)