#include "Core/RefCount.h"
#include "Core/Asserts.h"

namespace Insight
{
    namespace Core
    {
        RefCount::RefCount()
            : m_count(0)
        { }

        RefCount::RefCount(const RefCount& other) NO_EXPECT
            : m_count(0)
        { }

        RefCount::RefCount(RefCount&& other) NO_EXPECT
        {
            m_count.store(other.GetReferenceCount());
        }

        RefCount::~RefCount()
        {
            ASSERT(GetReferenceCount() == 0);
        }

        RefCount RefCount::operator=(const RefCount& other) const
        {
            return *this;
        }

        RefCount RefCount::operator=(RefCount&& other)
        {
            m_count.store(other.GetReferenceCount());
            return *this;
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