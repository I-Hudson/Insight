#pragma once

#include "Core/IObject.h"

#include <atomic>

namespace Insight
{
    template<typename>
    class ReferencePtr;

    namespace Core
    {
        /// @brief 
        class IS_CORE RefCount : public IObject
        {
            static_assert(std::atomic<u32>::is_always_lock_free);
        public:
            RefCount();
            RefCount(const RefCount& other);
            RefCount(RefCount&& other);
            ~RefCount();

            RefCount operator=(const RefCount& other) const;
            RefCount operator=(RefCount&& other);

            u32 GetReferenceCount() const;

            u32 Reference();
            u32 Unreference();

        private:
            std::atomic<u32> m_count;

            template<typename>
            friend class ReferencePtr;
        };
    } 
}