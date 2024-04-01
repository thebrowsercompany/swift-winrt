// The real weakreference.h will provide different definitions in C vs C++ compilation mode.
// Override it with ours so we ensure we get the C definitions.

#include <objidl.h>
#include <inspectable.h>

#ifndef _IWEAKREFERENCE_DEFINED
#define _IWEAKREFERENCE_DEFINED
typedef interface IWeakReference IWeakReference;
#endif

typedef struct IWeakReferenceVtbl
{
    BEGIN_INTERFACE

    HRESULT (STDMETHODCALLTYPE* QueryInterface)(__RPC__in IWeakReference* This,
        REFIID riid,
        _COM_Outptr_ void** ppvObject);
    ULONG (STDMETHODCALLTYPE* AddRef)(__RPC__in IWeakReference* This);
    ULONG (STDMETHODCALLTYPE* Release)(__RPC__in IWeakReference* This);
    HRESULT (STDMETHODCALLTYPE* Resolve)(__RPC__in IWeakReference* This,
        __RPC__in REFIID riid,
        __RPC__deref_out_opt IInspectable **objectReference);

    END_INTERFACE
} IWeakReferenceVtbl;

interface IWeakReference
{
    CONST_VTBL struct IWeakReferenceVtbl* lpVtbl;
};

#ifndef _IWEAKREFERENCESOURCE_DEFINED
#define _IWEAKREFERENCESOURCE_DEFINED
typedef interface IWeakReferenceSource IWeakReferenceSource;
#endif

typedef struct IWeakReferenceSourceVtbl
{
    BEGIN_INTERFACE

    HRESULT (STDMETHODCALLTYPE* QueryInterface)(__RPC__in IWeakReferenceSource* This,
        REFIID riid,
        _COM_Outptr_ void** ppvObject);
    ULONG (STDMETHODCALLTYPE* AddRef)(__RPC__in IWeakReferenceSource* This);
    ULONG (STDMETHODCALLTYPE* Release)(__RPC__in IWeakReferenceSource* This);
    HRESULT (STDMETHODCALLTYPE* GetWeakReference)(__RPC__in IWeakReferenceSource* This,
        __RPC__deref_out_opt IWeakReference **weakReference);

    END_INTERFACE
} IWeakReferenceSourceVtbl;

interface IWeakReferenceSource
{
    CONST_VTBL struct IWeakReferenceSourceVtbl* lpVtbl;
};
