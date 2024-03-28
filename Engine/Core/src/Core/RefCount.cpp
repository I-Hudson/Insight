#include "Core/RefCount.h"

namespace Insight
{
    namespace Core
    {
        RefCount::RefCount()
            : m_count(0)
        {
        }

        RefCount::~RefCount()
        {
            ASSERT(GetReferenceCount() == 0);
        }

        u32 RefCount::GetReferenceCount() const
        {
            return m_count.load(std::memory_order_acquire);
        }

        u32 RefCount::Reference()
        {
            return m_count.fetch_add(1, std::memory_order_acq_rel) + 1;
        }

        u32 RefCount::Unreference()
        {
            return m_count.fetch_sub(1, std::memory_order_acq_rel) - 1;
        }
    }
}