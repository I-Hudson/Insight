#pragma once

#include "Core/TypeAlias.h"
#include "Core/Defines.h"

#include "Platforms/Platform.h"

#include <vector>
#include <array>
#include <string>

namespace Insight
{
    template<typename T>
    class Span
    {
        using ElementType = T;
        using ValueType = std::remove_cv_t<T>;
        using Pointer = T*;
        using ConstPointer = const T*;
        using Reference = T&;
        using ConstReference = const T&;
        using Iterator = Pointer;
        using ConstIterator = ConstPointer;
        using ReverseIterator = std::reverse_iterator<Iterator>;
        using ConstReverseIterator = std::reverse_iterator<ConstReference>;
        using SizeType = size_t;
        using DifferenceType = ptrdiff_t;

#define OUT_OF_RANGE(index) ASSERT(index < m_size)

    public:
        Span() { }
        constexpr Span(Pointer ptr, SizeType size)
        {
            m_ptr = ptr; m_size = size;
        }
        Span(std::vector<ElementType> const& vec)
        {
            m_ptr = RemoveConst(vec.data()); m_size = vec.size();
        }
        template<u64 Size>
        Span(std::array<ElementType, Size> const& array)
        {
            m_ptr = RemoveConst(array.data()); m_size = array.size();
        }
        Span(std::initializer_list<ElementType> const& initList)
        {
            m_ptr = RemoveConst(initList.begin()); m_size = initList.size();
        }
        Span(std::string const& str)
        {
            m_ptr = RemoveConst(str.data()); m_size = str.size();
        }

        constexpr Span(const Span<ElementType>& other)
        {
            m_ptr = other.m_ptr; m_size = other.m_size;
        }
        constexpr Span(Span<ElementType>&& other)
        {
            m_ptr = other.m_ptr; m_size = other.m_size; other.m_ptr = nullptr; other.m_size = 0;
        }

        ~Span() 
        {
            m_ptr = nullptr; m_size = 0;
        }

        Span<ElementType>& operator=(const Span<ElementType>& other) const
        {
            m_ptr = other.m_ptr; m_size = other.m_size;
        }
        Span<ElementType>& operator=(Span<ElementType>&& other) const
        {
            m_ptr = other.m_ptr; m_size = other.m_size; other.m_ptr = nullptr; other.m_size = 0;
        }

        bool operator==(const Span<ElementType>& other) const
        {
            return m_ptr == other.m_ptr
                && m_size == other.m_size;
        }
        bool operator!=(const Span<ElementType>& other) const
        {
            return !(*this == other);
        }

        bool operator>(const Span<ElementType>& other) const = delete;
        bool operator>=(const Span<ElementType>& other) const = delete;
        bool operator<(const Span<ElementType>& other) const = delete;
        bool operator<=(const Span<ElementType>& other) const = delete;

        constexpr bool IsEmpty()                    const { return m_size == 0; }
        operator bool()                             const { return IsEmpty(); }

        constexpr u64 Size()                        const { return m_size; }

        constexpr Pointer Data()                    const { return m_ptr; }
        constexpr Pointer Front()                   const { return m_ptr; }
        constexpr Pointer Back()                    const { return At(m_size); }

        constexpr Reference operator[](u32 index)   const { return At(index); }
        constexpr Reference At(u32 index)           const { OUT_OF_RANGE(index); return *(Data() + index); }

        constexpr Iterator Begin()                  const { return Front(); }
        constexpr ConstIterator CBegin()            const { return Front(); }
        constexpr Iterator End()                    const { return Back(); }
        constexpr ConstIterator CEnd()              const { return Back(); }

    private:

    private:
        Pointer m_ptr = nullptr;
        u64 m_size = 0;
#undef OUT_OF_RANGE
    };
}