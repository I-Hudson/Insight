#pragma once

#include "Core/TypeAlias.h"

#include "Graphics/RHI/RHI_Handle.h"

#include <vector>
#include <stack>
#include <mutex>

namespace Insight
{
    namespace Graphics
    {
        /// @brief Structure to store a handle to a graphics API object (Hot Type) for a resource (like a texture, buffer, shader, root signature, etc)
        /// and additional information about it (Cold Type).
        /// @tparam HotType (frequently accessed)
        /// @tparam ColdType (not frequently accessed)
        template<typename HotType, typename ColdType, u64 InitialSize = 128ull>
        class RHI_ResourcePool
        {
        public:
            RHI_ResourcePool() 
            {
                std::lock_guard hotLock(m_hotMutex);
                std::lock_guard coldLock(m_coldMutex);
                std::lock_guard generationLock(m_generationMutex);
                std::lock_guard freeListLock(m_freeListMutex);

                m_hot.resize(InitialSize);
                m_cold.resize(InitialSize);
                m_generation.resize(InitialSize);
                for (u64 i = 0; i < InitialSize; ++i)
                {
                    m_freeList.push(static_cast<u32>(i));
                }
            }
            ~RHI_ResourcePool() { }

            RHI_Handle<ColdType> Create(HotType& hotType, ColdType& coldType)
            {
                RHI_Handle<ColdType> handle;
                std::lock_guard freeListLock(m_freeListMutex);
                if (m_freeList.empty())
                {
                    Grow(m_hot.size() * 2);
                }
                const u32 freeIndex = m_freeList.top();
                m_freeList.pop();
                handle.Index = freeIndex;
                return handle;
            }

            void Release(const RHI_Handle<ColdType> handle, HotType& hotType, ColdType& coldType)
            {
                std::lock_guard freeListLock(m_freeListMutex);
                std::lock_guard generationLock(m_generationMutex);

                hotType = m_hot.at(handle.Index);
                coldType = m_cold.at(handle.Index);

                m_freeList.push(handle.Index);
                ++m_generation.at(handle.Index);
            }

            const HotType* GetHotType(const RHI_Handle<ColdType> handle) const
            {
                std::lock_guard hotLock(m_hotMutex);
                std::lock_guard generationLock(m_generationMutex);
                if (m_generation.at(handle.Index) != handle.Generation)
                {
                    return nullptr;
                }
                return &m_hot.at(handle.Index);
            }

            const ColdType* GetColdType(const RHI_Handle<ColdType> handle) const
            {
                std::lock_guard coldLock(m_coldMutex);
                std::lock_guard generationLock(m_generationMutex);
                if (m_generation.at(handle.Index) != handle.Generation)
                {
                    return nullptr;
                }
                return &m_cold.at(handle.Index);
            }


        private:
            void Grow(const u64 newSize)
            {
                u64 oldSize = 0;
                {
                    std::lock_guard hotLock(m_hotMutex);
                    std::lock_guard coldLock(m_coldMutex);
                    std::lock_guard generationLock(m_generationMutex);

                    oldSize = m_hot.size();
                    m_hot.resize(newSize);
                    m_cold.resize(newSize);
                    m_generation.resize(newSize);
                }

                const u64 difference = newSize - oldSize;
                for (u64 i = oldSize; i < difference; ++i)
                {
                    m_freeList.push(static_cast<u32>(i));
                }
            }

        private:
            std::vector<HotType> m_hot;
            std::vector<ColdType> m_cold;
            std::vector<u32> m_generation;
            std::stack<u32> m_freeList;
        
            mutable std::mutex m_hotMutex;
            mutable std::mutex m_coldMutex;
            mutable std::mutex m_generationMutex;
            mutable std::mutex m_freeListMutex;
        };
    }
}