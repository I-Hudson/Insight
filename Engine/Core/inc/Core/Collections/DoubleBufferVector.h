#pragma once

#include <vector>
#include <mutex>

namespace Insight
{
    template<typename T>
    class DoubleBufferVector
    {
        constexpr static const u32 c_MaxCapcity = 2;
    public:
        DoubleBufferVector()
        {
            m_data.resize(c_MaxCapcity);
        }

        u64 Size() const { return static_cast<u64>(c_MaxCapcity); }

        const T& operator=(const T& value)
        {
            std::lock_guard lock(m_mutex);
            const T& v = m_data[m_currentIndex] = value;
            return v;
        }

        const T& operator++()
        {
            std::lock_guard lock(m_mutex);
            const T& v = ++m_data[m_currentIndex];
            return v;
        }
        const T& operator++(int)
        {
            std::lock_guard lock(m_mutex);
            const T& v = ++m_data[m_currentIndex];
            return v;
        }
        const T& operator+=(const T& value)
        {
            std::lock_guard lock(m_mutex);
            const T& v = m_data[m_currentIndex] += value;
            return v;
        }

        const T& operator--()
        {
            std::lock_guard lock(m_mutex);
            const T& v = --m_data[m_currentIndex];
            return v;
        }
        const T& operator--(int)
        {
            std::lock_guard lock(m_mutex);
            const T& v = --m_data[m_currentIndex];
            return v;
        }
        const T& operator-=(const T& value)
        {
            std::lock_guard lock(m_mutex);
            const T& v = m_data[m_currentIndex] -= value;
            return v;
        }

        const T& GetCurrent() const
        {
            std::lock_guard lock(m_mutex);
            return m_data[m_currentIndex];
        }
        T& GetCurrent()
        {
            std::lock_guard lock(m_mutex);
            return m_data[m_currentIndex];
        }

        const T& GetPending() const
        {
            std::lock_guard lock(m_mutex);
            return m_data[m_currentIndex];
        }
        T& GetPending()
        {
            std::lock_guard lock(m_mutex);
            return m_data[!m_currentIndex];
        }

        void Swap()
        {
            std::lock_guard lock(m_mutex);
            m_currentIndex = (m_currentIndex + 1) % c_MaxCapcity;
        }

        void Clear()
        {
            std::lock_guard lock(m_mutex);
            m_data.clear();
        }

    private:
        u64 m_currentIndex = 0;
        mutable std::mutex m_mutex;
        std::vector<T> m_data;
    };
}