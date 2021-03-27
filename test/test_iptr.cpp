// test_iptr.cpp //////////////////////////////////////////////////////////////
//
// iptr is released under the MIT license.
// For details, see LICENSE.txt
//
// Tests for iptr.h
//

#include <iptr.h>
#include <Windows.h>
#include <iostream>

// Simulate COM interfaces

#undef INTERFACE

#define INTERFACE IA
DECLARE_INTERFACE_IID_(IA, IUnknown,
                       "B92F633A-8E96-11EB-B727-DC41A9695036")
{
    BEGIN_INTERFACE
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;
    STDMETHOD(Method1)(THIS_ const wchar_t *message) PURE;
    END_INTERFACE
};
#undef INTERFACE

#define INTERFACE IB
DECLARE_INTERFACE_IID_(IB, IUnknown,
                       "B92F633B-8E96-11EB-B727-DC41A9695036")
{
    BEGIN_INTERFACE
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;
    STDMETHOD(Method2)(THIS_ IA *in) PURE;
    STDMETHOD(Method3)(THIS_ IA **out) PURE;
    END_INTERFACE
};
#undef INTERFACE

class CAB : public IA, public IB {
    friend HRESULT NewCAB(REFIID, void**);
    ULONG m_rc;
    CAB() : m_rc(0) { std::wcout << L"Creating CAB: " << this << std::endl; }
public:
    virtual ~CAB() { std::wcout << L"Destroying CAB: " << this << std::endl; }

    STDMETHODIMP QueryInterface(REFIID riid, void **ppv) noexcept override
    {
        if(!ppv) return E_INVALIDARG;
        if(riid == IID_IUnknown || riid == __uuidof(IA))
            *ppv = static_cast<IA*>(this);
        else if(riid == __uuidof(IB))
            *ppv = static_cast<IB*>(this);
        else return (*ppv = nullptr), E_NOINTERFACE;
        reinterpret_cast<IUnknown*>(this)->AddRef();
        return S_OK;
    }

    STDMETHODIMP_(ULONG) AddRef() noexcept override
    {
        return ++m_rc;
    }

    STDMETHODIMP_(ULONG) Release() noexcept override
    {
        long rc = --m_rc;
        if(rc == 0) delete this;
        return rc;
    }

    STDMETHODIMP Method1(const wchar_t *message) noexcept override
    {
        if(!message) return E_INVALIDARG;
        std::wcout << L"IA::Method1: " << message << std::endl;
        return S_OK;
    }

    STDMETHODIMP Method2(IA *in) noexcept override
    {
        if(!in) return E_INVALIDARG;
        in->Method1(L"IB::Method3");
        return S_OK;
    }

    STDMETHODIMP Method3(IA **out) noexcept override
    {
        if(!out) return E_POINTER;
        *out = nullptr;
        std::wcout << L"IB::Method4: Spawning a new CAB" << std::endl;
        try
        {
            auto p = new CAB();
            p->AddRef();
            HRESULT hr = p->QueryInterface(__uuidof(IA),
                                           reinterpret_cast<void**>(out));
            p->Release();
            return hr;
        }
        catch(std::bad_alloc&)
        {
            return E_OUTOFMEMORY;
        }
        catch(...)
        {
            return E_UNEXPECTED;
        }
    }
};

HRESULT NewCAB(REFIID riid, void **ppv)
{
    try
    {
        auto p = new CAB;
        p->AddRef();
        HRESULT hr = p->QueryInterface(riid, ppv);
        p->Release();
        return hr;
    }
    catch(std::bad_alloc&)
    {
        return E_OUTOFMEMORY;
    }
    catch(...)
    {
        return E_FAIL;
    }
}

int main()
{
    HRESULT hr;
    IPtr::IPtr<IA> p1;
    hr = NewCAB(__uuidof(IA), reinterpret_cast<void**>(set(p1)));
    assert(SUCCEEDED(hr));
    assert(p1);

    hr = p1->Method1(L"Hello, world!");
    assert(SUCCEEDED(hr));

    // Try "As" with an interface the CAB object implements
    auto p2 = p1.As<IB>();
    assert(p2);

    // Try "As" with an interface that the CAB object does not implement
    auto p3 = p1.As<ISupportErrorInfo>();
    assert(!p3);

    // Also try getting the interfaces via QueryInterface
    IPtr::IPtr<IA> p4;
    hr = p2->QueryInterface(
            __uuidof(IA),
            reinterpret_cast<void**>(set(p4)));

    assert(SUCCEEDED(hr));
    assert(p4);

    IPtr::IPtr<IB> p5;
    hr = p1->QueryInterface(
            __uuidof(IB),
            reinterpret_cast<void**>(set(p5)));

    assert(SUCCEEDED(hr));
    assert(p5);

    // Call an IB method that takes an interface pointer
    hr = p5->Method2(get(p1));
    assert(SUCCEEDED(hr));

    hr = p2->Method2(get(p1));
    assert(SUCCEEDED(hr));

    // Call IB method that returns an interface pointer
    IPtr::IPtr<IA> p6;
    hr = p2->Method3(set(p6));
    assert(SUCCEEDED(hr));
    assert(p6);

    // Equality operator (p1 and p4 point to the same object)
    assert(p1 == p4);

    // Non-equality operator (p1 and p6 point to different objects)
    assert(p1 != p6);

    // Other comparisons
    if(p1 < p6)
    {
        assert(p6 > p1);
        assert(p6 >= p1);
        assert(!(p6 <= p1));
    }
    else
    {
        assert(p1 > p6);
        assert(p1 >= p6);
        assert(!(p1 <= p6));
    }

    // Copy construction (same interface type)
    auto p7(p6);
    p7->Method1(L"I came from a copy constructor");

    // Copy construction (different, but compatible, interface type)
    IPtr::IPtr<IUnknown> p8(p7);
    assert(p8);
    p8.As<IA>()->Method1(L"I came from a different copy constructor");

    // Move construction
    IPtr::IPtr<IA> p9(std::move(p7));
    assert(!p7); // NOLINT
    assert(p9);
    p9->Method1(L"I came from a move constructor");

    // Copy Assignment
    IPtr::IPtr<IA> p10;
    assert(!p10);
    p10 = p1;
    assert(p10);
    p10->Method1(L"I came from copy assignment");

    // Template copy assignment (different, but compatible, interface type)
    p8 = p10;
    assert(p8);
    assert(p10);

    // Assign nullptr
    p8 = nullptr;
    assert(!p8);

    // Move assignment
    p9 = std::move(p10);
    assert(!p10); // NOLINT
    p9->Method1(L"I came from move assignment");

    // Detach
    IA* ia = detach(p9);
    assert(ia);
    assert(!p9);

    // Attach
    attach(p9, ia);
    ia = nullptr;
    assert(p9);
    p9->Method1(L"I came from a attaching to a raw pointer");

    // Swap
    assert(p9);
    assert(!p10);
    swap(p9, p10);
    assert(!p9);
    assert(p10);

    // CopyTo: Hand out another reference to the same interface without calling
    // QueryInterface
    p10.CopyTo(set(p9));
    assert(p9);
    p9->Method1(L"I came from CopyTo");

    // Make a copy of an existing pointer that we would like to hold on to
    // (i.e., we want a copy of the pointer even after ia is released)
    assert(!ia);
    hr = p2->QueryInterface(__uuidof(IA), reinterpret_cast<void**>(&ia));
    assert(SUCCEEDED(hr));
    assert(ia);
    IPtr::IPtr<IA> p11;
    p11.CopyFrom(ia);
    ia->Release();
    ia = nullptr;
    assert(p11);
    p11->Method1(L"I came from CopyFrom");

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
