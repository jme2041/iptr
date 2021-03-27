// iptr.h /////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2021, Jeffrey M. Engelmann
//
// iptr is released under the MIT license.
// For details, see LICENSE.txt
//
// IPtr: Smart pointer for COM interfaces
//
// IPtr is a non-Windows Runtime version of ComPtr, first implemented by Kenny
// Kerr and now incorporated into the Windows Runtime library (client.h).
// The original ComPtr was also released under the MIT license.
//

#ifndef IPTR_H
#define IPTR_H

#include <cstddef>
#include <cassert>

#ifdef IPTR_SHOW_TRACE
#include <iostream>
#define IPTR_TRACE(s) std::cout << "IPtr trace: " << s << std::endl
#else
#define IPTR_TRACE(s) ((void)0)
#endif

namespace IPtr {

    // This class hides AddRef() and Release()
    template<typename T>
    class NARR : public T {
        unsigned long __stdcall AddRef();
        unsigned long __stdcall Release();
    };

    // IPtr: Interface pointer wrapper. T is a COM interface.
    template<typename T>
    class IPtr {

        // U is a COM interface
        template<typename U>
        friend class IPtr;

        T *m_ptr = nullptr;

        void InternalAddRef() const noexcept
        {
            if(m_ptr) m_ptr->AddRef();
        }

        void InternalRelease() noexcept
        {
            T *temp = m_ptr;
            if(temp)
            {
                m_ptr = nullptr;
                temp->Release();
            }
        }

        void InternalCopy(T *other) noexcept
        {
            if(m_ptr != other)
            {
                InternalRelease();
                m_ptr = other;
                InternalAddRef();
            }
        }

        template<typename U>
        void InternalMove(IPtr<U>& other) noexcept
        {
            if(m_ptr != other.m_ptr)
            {
                InternalRelease();
                m_ptr = other.m_ptr;
                other.m_ptr = nullptr;
            }
        }

    public:
        IPtr() noexcept = default;

        IPtr(const IPtr& other) noexcept : m_ptr(other.m_ptr)
        {
            IPTR_TRACE("Copy constructor");
            InternalAddRef();
        }

        template<typename U>
        explicit IPtr(const IPtr<U>& other) noexcept : m_ptr(other.m_ptr)
        {
            IPTR_TRACE("Template copy constructor");
            InternalAddRef();
        }

        template<typename U>
        explicit IPtr(IPtr<U>&& other) noexcept : m_ptr(other.m_ptr)
        {
            IPTR_TRACE("Move constructor");
            other.m_ptr = nullptr;
        }

        ~IPtr() noexcept
        {
            IPTR_TRACE("Destructor");
            InternalRelease();
        }

        IPtr& operator=(const IPtr& other) noexcept // NOLINT
        {
            IPTR_TRACE("Copy assignment");
            InternalCopy(other.m_ptr);
            return *this;
        }

        template<typename U>
        IPtr& operator=(const IPtr<U>& other) noexcept
        {
            IPTR_TRACE("Template copy assignment");
            InternalCopy(other.m_ptr);
            return *this;
        }

        template<typename U>
        IPtr& operator=(IPtr<U>&& other) noexcept
        {
            IPTR_TRACE("Move assignment");
            InternalMove(other);
            return *this;
        }

        IPtr& operator=(std::nullptr_t) noexcept
        {
            IPTR_TRACE("nullptr assignment");
            InternalRelease();
            return *this;
        }

        explicit operator bool() const noexcept
        {
            return m_ptr != nullptr;
        }

        NARR<T> * operator->() const noexcept
        {
            return static_cast<NARR<T>*>(m_ptr);
        }

        friend T * get(const IPtr& obj) noexcept
        {
            return obj.m_ptr;
        }

        friend T ** set(IPtr& obj) noexcept
        {
            assert(obj.m_ptr == nullptr);
            return &obj.m_ptr;
        }

        friend void attach(IPtr& obj, T *p) noexcept
        {
            obj.InternalRelease();
            obj.m_ptr = p;
        }

        friend T * detach(IPtr& obj) noexcept
        {
            T * temp = obj.m_ptr;
            obj.m_ptr = nullptr;
            return temp;
        }

        friend void swap(IPtr& left, IPtr& right) noexcept
        {
            T *temp = left.m_ptr;
            left.m_ptr = right.m_ptr;
            right.m_ptr = temp;
        }

        template<typename U>
        IPtr<U> As() const noexcept
        {
            IPtr<U> temp;
            m_ptr->QueryInterface(
                    __uuidof(U),
                    reinterpret_cast<void**>(set(temp)));
            return temp;
        }

        void CopyFrom(T *other) noexcept
        {
            InternalCopy(other);
        }

        void CopyTo(T **other) noexcept
        {
            InternalAddRef();
            *other = m_ptr;
        }
    };

    template<typename T>
    bool operator==(const IPtr<T>& left, const IPtr<T>& right) noexcept
    {
        return get(left) == get(right);
    }

    template<typename T>
    bool operator!=(const IPtr<T>& left, const IPtr<T>& right) noexcept
    {
        return !(left == right);
    }

    template<typename T>
    bool operator<(const IPtr<T>& left, const IPtr<T>& right) noexcept
    {
        return get(left) < get(right);
    }

    template<typename T>
    bool operator>(const IPtr<T>& left, const IPtr<T>& right) noexcept
    {
        return right < left;
    }

    template<typename T>
    bool operator<=(const IPtr<T>& left, const IPtr<T>& right) noexcept
    {
        return !(right < left);
    }

    template<typename T>
    bool operator>=(const IPtr<T>& left, const IPtr<T>& right) noexcept
    {
        return !(left < right);
    }
}

#endif // IPTR_H

///////////////////////////////////////////////////////////////////////////////
