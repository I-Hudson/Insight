#pragma once

#include "Core/RefCount.h"
#include <typeinfo>

namespace Insight
{
    template<typename TOther, typename T>
    constexpr bool rp_is_convertible_v = std::is_convertible_v<TOther*, T*>;

    template<typename T>
    class ReferencePtr
    {
        static_assert(std::is_base_of_v<Core::RefCount, T>);
    public:
        ReferencePtr() { }
        ReferencePtr(std::nullptr_t) { }
        ReferencePtr(T* pointer)
        {
            Set(pointer);
        }
        template<typename TOther, std::enable_if_t<rp_is_convertible_v<TOther, T>, int> = 0>
        ReferencePtr(TOther* pointer)
        {
            Set(pointer);
        }

        ReferencePtr(const ReferencePtr& other)
        {
            Set(other.m_pointer);
        }
        template<typename TOther, std::enable_if_t<rp_is_convertible_v<TOther, T>, int> = 0>
        ReferencePtr(const ReferencePtr<TOther>& other)
        {
            Set(other.m_pointer);
        }
        ~ReferencePtr() { Reset(); }

        operator bool() const
        {
            return m_pointer != nullptr;
        }

        ReferencePtr operator=(const ReferencePtr& other)
        {
            ReferencePtr(other).Swap(*this);
            return *this;
        }
        template<typename TOther, std::enable_if_t<rp_is_convertible_v<TOther, T>, int> = 0>
        ReferencePtr operator=(const ReferencePtr<TOther>& other)
        {
            ReferencePtr<TOther>(other).Swap(*this);
            return *this;
        }

        void Reset()
        {
            Set(nullptr);
        }

        template<typename TOther>
        ReferencePtr<TOther> As()
        {
            ReferencePtr<TOther> referencePtr(dynamic_cast<TOther*>(m_pointer));
            return referencePtr;
        }

        T* Ptr() { return m_pointer; }
        const T* Ptr() const { return m_pointer; }

    private:
        void Delete()
        {
            if (m_pointer)
            {
                ::Delete(m_pointer);
            }
        }

        void Set(T* pointer)
        {
            if (m_pointer)
            {
                if (m_pointer->Unreference() == 0)
                {
                    Delete();
                }
                m_pointer = nullptr;
            }

            if (pointer)
            {
                m_pointer = pointer;
                m_pointer->Reference();
            }
        }

        template<typename TOther>
        void Swap(ReferencePtr<TOther>& other)
        {
            T* temp = m_pointer;
            m_pointer = other.m_pointer;
            other.m_pointer = temp;
        }

    private:
        T* m_pointer = nullptr;

        template<typename>
        friend class ReferencePtr;
    };

    template<typename T>
    using Ref = ReferencePtr<T>;

    template<typename T>
    bool operator==(std::nullptr_t, const ReferencePtr<T>& right) 
    { return nullptr == right.Ptr(); }
    
    template<typename T>
    bool operator==(const ReferencePtr<T>& left, std::nullptr_t) 
    { return left.ptr() == nullptr; }

    template<typename TOther1, typename TOther2>
    bool operator==(const ReferencePtr<TOther1>& left, const ReferencePtr<TOther2>& right)
    {
        return left.Ptr() == right.Ptr();
    }

    template<typename T>
    bool operator!=(std::nullptr_t, const ReferencePtr<T>& right)
    {
        return nullptr != right.Ptr();
    }

    template<typename T>
    bool operator!=(const ReferencePtr<T>& left, std::nullptr_t)
    {
        return left..Ptr() != nullptr;
    }

    template<typename TOther1, typename TOther2>
    bool operator!=(const ReferencePtr<TOther1>& left, const ReferencePtr<TOther2>& right)
    {
        return left.Ptr() != right.Ptr();
    }
}