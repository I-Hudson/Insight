#if defined(IS_DX12_ENABLED)

#include "Graphics/RHI/DX12/RHI_GPUMeshAllocator_DX12.h"
#include "Graphics/RHI/DX12/RenderContext_DX12.h"
#include "Graphics/RHI/DX12/RHI_Buffer_DX12.h"
#include "Graphics/RHI/RHI_UploadQueue.h"

#include "Threading/ScopedLock.h"

#include "Core/Logger.h"
#include "Platforms/Platform.h"

namespace Insight
{
    namespace Graphics
    {
        namespace RHI::DX12
        {
            RHI_GPUMeshAllocator_DX12::~RHI_GPUMeshAllocator_DX12()
            {
            }

            void RHI_GPUMeshAllocator_DX12::Initialise(RenderContext* renderContext)
            {
                ASSERT(renderContext);
                m_renderContext = static_cast<RenderContext_DX12*>(renderContext);

                RHI_Buffer_Overrides bufferOverride = {};
                bufferOverride.AllowUnorderedAccess = true;

                m_gpuMeshBuffer = static_cast<RHI_Buffer_DX12*>(Renderer::CreateRawBuffer(1_GB, bufferOverride));
                ASSERT(m_gpuMeshBuffer != nullptr);

                D3D12MA::VIRTUAL_BLOCK_DESC blockDesc = {};
                blockDesc.Flags = D3D12MA::VIRTUAL_BLOCK_FLAG_NONE;
                blockDesc.Size = m_gpuMeshBuffer->GetSize();

                ThrowIfFailed(D3D12MA::CreateVirtualBlock(&blockDesc, &m_meshBufferAllocator));
            }

            void RHI_GPUMeshAllocator_DX12::Shutdown()
            {
                D3D12MA::Statistics stats;
                m_meshBufferAllocator->GetStatistics(&stats);

                ASSERT(stats.AllocationCount == 0);
                m_meshBufferAllocator->Release();
            }

            RHI_GPUMeshAllocatorHandle RHI_GPUMeshAllocator_DX12::AllocateMeshLOD(void* vertexData, u64 vertexSize, void* indexData, u64 indexSize, GPU_MeshLODMetaData& meshLODMetadata)
            {
                Threading::ScopedLock lock(m_lock);

                const u64 totalSize = vertexSize + indexSize;
                D3D12MA::DetailedStatistics stats;
                m_meshBufferAllocator->CalculateStatistics(&stats);

                const u64 freeSpaceBytes = stats.AllocationSizeMax;

                if (totalSize > freeSpaceBytes)
                {
                    IS_LOG_CORE_ERROR("[RHI_GPUMeshAllocator_DX12::AllocateMeshLOD] Unable to allocate {} bytes.", totalSize);
                    return RHI_GPUMeshAllocatorHandle();
                }
                else
                {
                    D3D12MA::VIRTUAL_ALLOCATION_DESC allocDesc = {};
                    allocDesc.Flags = D3D12MA::VIRTUAL_ALLOCATION_FLAG_NONE;
                    allocDesc.Size = totalSize;

                    u64 allocationOffset = 0;
                    D3D12MA::VirtualAllocation allocation = {};
                    ThrowIfFailed(m_meshBufferAllocator->Allocate(&allocDesc, &allocation, &allocationOffset));

                    const u64 dstVertexOffset = allocationOffset;
                    const u64 dstIndexOffset = allocationOffset + vertexSize;

                    meshLODMetadata.VertexBufferAddress = m_gpuMeshBuffer->GetResource()->GetGPUVirtualAddress() + dstVertexOffset;
                    meshLODMetadata.IndexBufferAddress = m_gpuMeshBuffer->GetResource()->GetGPUVirtualAddress() + dstIndexOffset;

                    m_renderContext->GetUploadQueue().UploadBuffer(vertexData, vertexSize, dstVertexOffset, 0, m_gpuMeshBuffer);
                    m_renderContext->GetUploadQueue().UploadBuffer(indexData, indexSize, dstIndexOffset, 0, m_gpuMeshBuffer);

                    RHI_GPUMeshAllocatorHandle handle = GetNextFreeSlot();
                    m_allocations[handle] = allocation;
                    return handle;
                }
            }

            void RHI_GPUMeshAllocator_DX12::FreeMeshLOD(RHI_GPUMeshAllocatorHandle handle)
            {
                Threading::ScopedLock lock(m_lock);
                if (auto iter = m_allocations.find(handle);
                    iter != m_allocations.end())
                {
                    m_meshBufferAllocator->FreeAllocation(iter->second);
                }
            }
        }
    }
}
#endif // defined(IS_DX12_ENABLED)