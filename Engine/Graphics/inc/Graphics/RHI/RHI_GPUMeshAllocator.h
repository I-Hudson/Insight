#pragma once

#include "Core/TypeAlias.h"
#include "Core/GUID.h"
#include "Core/Profiler.h"

#include "Threading/SpinLock.h"

#include <vector>
#include <queue>

namespace Insight
{
    namespace Graphics
    {
        class RenderContext;
        class RHI_Buffer;

        struct RHI_GPUMeshAllocatorHandle
        {
            RHI_GPUMeshAllocatorHandle()
            { }

            bool IsValid() const { return m_metadataIndex != -1; }
            operator bool() const { return IsValid(); }

            bool operator==(const RHI_GPUMeshAllocatorHandle& other) const { return m_metadataIndex == other.m_metadataIndex; }
            bool operator!=(const RHI_GPUMeshAllocatorHandle& other) const { return m_metadataIndex != other.m_metadataIndex; }
            
        private:
            RHI_GPUMeshAllocatorHandle(u32 metadataIndex)
                : m_metadataIndex(metadataIndex)
            { }

        private:
            u32 m_metadataIndex = -1;

            friend class RHI_GPUMeshAllocator;
            template<typename T>
            friend struct ::std::hash;
        };

        struct GPU_MeshLODMetaData
        {
            u64 VertexBufferAddress = 0;
            u64 IndexBufferAddress = 0;

            u32 VertexOffset = 0;
            u32 VertexStride = 0;
            u32 IndexCount = 0;

            u16 MaterialIndex = 0;
            u16 DebugLodLevel = 0;

            float BoundCentre[3];
            float BoundingRadius;
        };

        /// @brief Manager and Allocator for all mesh and mesh metadata in the engine.
        class RHI_GPUMeshAllocator
        {
        public:
            virtual ~RHI_GPUMeshAllocator() {}

            virtual void Initialise(RenderContext* renderContext) = 0;
            virtual void Shutdown() = 0;

            virtual RHI_GPUMeshAllocatorHandle AllocateMeshLOD(void* vertexData, u64 vertexSize, void* indexData, u64 indexSize, GPU_MeshLODMetaData& meshLODMetadata) = 0;
            virtual void FreeMeshLOD(RHI_GPUMeshAllocatorHandle handle) = 0;

        protected:
            RHI_GPUMeshAllocatorHandle GetNextFreeSlot();
            void FreeSlot(RHI_GPUMeshAllocatorHandle handle);

        protected:
            IS_PROFILE_LOCKABLE_NAME(Threading::SpinLock, m_lock, "RHI_GPUMeshAllocatorLock");
            std::vector<GPU_MeshLODMetaData> m_meshMetadata;
            std::queue<u32> m_meshMetadataFreeSlots;
        };
    }
}

namespace std
{
    template<>
    struct hash<Insight::Graphics::RHI_GPUMeshAllocatorHandle>
    {
        u64 operator()(Insight::Graphics::RHI_GPUMeshAllocatorHandle handle) const
        {
            std::hash<u64> hash;
            return hash(handle.m_metadataIndex);
        }
    };
}