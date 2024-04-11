#pragma once

#include "Graphics/Defines.h"
#include "Core/TypeAlias.h"

namespace Insight
{
    namespace Graphics
    {
        struct IS_GRAPHICS RHI_MemoryInfo
        {
            /** \brief Number of D3D12 memory blocks allocated - `ID3D12Heap` objects and committed resources.
            */
            u32 BlockCount;
            /** \brief Number of D3D12MA::Allocation objects allocated.

            Committed allocations have their own blocks, so each one adds 1 to `AllocationCount` as well as `BlockCount`.
            */
            u32 AllocationCount;
            /** \brief Number of bytes allocated in memory blocks.
            */
            u64 BlockBytes;
            /** \brief Total number of bytes occupied by all D3D12MA::Allocation objects.

            Always less or equal than `BlockBytes`.
            Difference `(BlockBytes - AllocationBytes)` is the amount of memory allocated from D3D12
            but unused by any D3D12MA::Allocation.
            */
            u64 AllocationBytes;

            /** \brief Estimated current memory usage of the program.

            Fetched from system using `IDXGIAdapter3::QueryVideoMemoryInfo` if possible.
            
            It might be different than `BlockBytes` (usually higher) due to additional implicit objects
            also occupying the memory, like swapchain, pipeline state objects, descriptor heaps, command lists, or
            heaps and resources allocated outside of this library, if any.
            */
            u64 UsageBytes;
            /** \brief Estimated amount of memory available to the program.

            Fetched from system using `IDXGIAdapter3::QueryVideoMemoryInfo` if possible.

            It might be different (most probably smaller) than memory capacity returned
            by D3D12MA::Allocator::GetMemoryCapacity() due to factors
            external to the program, decided by the operating system.
            Difference `BudgetBytes - UsageBytes` is the amount of additional memory that can probably
            be allocated without problems. Exceeding the budget may result in various problems.
            */
            u64 BudgetBytes;
        };
    }
}