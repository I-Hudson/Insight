#include "Graphics/RHI/RHI_GPUMeshAllocator.h"

#include "Threading/ScopedLock.h"

namespace Insight
{
    namespace Graphics
    {
        RHI_GPUMeshAllocatorHandle RHI_GPUMeshAllocator::GetNextFreeSlot()
        {
            if (m_meshMetadataFreeSlots.empty())
            {
                m_meshMetadataFreeSlots.push(m_meshMetadata.size());
                m_meshMetadata.push_back({});
            }

            const u32 slot = m_meshMetadataFreeSlots.front();
            m_meshMetadataFreeSlots.pop();

            return RHI_GPUMeshAllocatorHandle(slot);
        }

        void RHI_GPUMeshAllocator::FreeSlot(RHI_GPUMeshAllocatorHandle handle)
        {
            m_meshMetadataFreeSlots.push(handle.m_metadataIndex);
        }
    }
}