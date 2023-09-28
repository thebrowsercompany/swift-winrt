// The real ivectorchangedeventargs.h will provide different definitions in C vs C++ compilation mode.
// Override it with ours so we ensure we get the C definitions.
#pragma once

#include <windows.h>
#include <inspectable.h>

#if !defined(____x_ABI_CWindows_CFoundation_CCollections_CIVectorChangedEventArgs_INTERFACE_DEFINED__)
#define ____x_ABI_CWindows_CFoundation_CCollections_CIVectorChangedEventArgs_INTERFACE_DEFINED__
typedef interface __x_ABI_CWindows_CFoundation_CCollections_CIVectorChangedEventArgs __x_ABI_CWindows_CFoundation_CCollections_CIVectorChangedEventArgs;

typedef /* [v1_enum] */
enum __x_ABI_CWindows_CFoundation_CCollections_CCollectionChange
{
    CollectionChange_Reset,
    CollectionChange_ItemInserted,
    CollectionChange_ItemRemoved,
    CollectionChange_ItemChanged,
} __x_ABI_CWindows_CFoundation_CCollections_CCollectionChange;

/* [contract, version, pointer_default(unique), uuid("575933df-34fe-4480-af15-07691f3d5d9b"), object] */
typedef struct __x_ABI_CWindows_CFoundation_CCollections_CIVectorChangedEventArgsVtbl
{
    BEGIN_INTERFACE

    HRESULT ( STDMETHODCALLTYPE *QueryInterface)(
        __RPC__in __x_ABI_CWindows_CFoundation_CCollections_CIVectorChangedEventArgs * This,
        /* [in] */ __RPC__in REFIID riid,
        /* [annotation][iid_is][out] */
        _COM_Outptr_  void **ppvObject
        );

    ULONG ( STDMETHODCALLTYPE *AddRef )(
        __RPC__in __x_ABI_CWindows_CFoundation_CCollections_CIVectorChangedEventArgs * This
        );

    ULONG ( STDMETHODCALLTYPE *Release )(
        __RPC__in __x_ABI_CWindows_CFoundation_CCollections_CIVectorChangedEventArgs * This
        );

    HRESULT ( STDMETHODCALLTYPE *GetIids )(
        __RPC__in __x_ABI_CWindows_CFoundation_CCollections_CIVectorChangedEventArgs * This,
        /* [out] */ __RPC__out ULONG *iidCount,
        /* [size_is][size_is][out] */ __RPC__deref_out_ecount_full_opt(*iidCount) IID **iids
        );

    HRESULT ( STDMETHODCALLTYPE *GetRuntimeClassName )(
        __RPC__in __x_ABI_CWindows_CFoundation_CCollections_CIVectorChangedEventArgs * This,
        /* [out] */ __RPC__deref_out_opt HSTRING *className
        );

    HRESULT ( STDMETHODCALLTYPE *GetTrustLevel )(
        __RPC__in __x_ABI_CWindows_CFoundation_CCollections_CIVectorChangedEventArgs * This,
        /* [OUT ] */ __RPC__out TrustLevel *trustLevel
        );

    /* [propget] */HRESULT ( STDMETHODCALLTYPE *get_CollectionChange )(
        __x_ABI_CWindows_CFoundation_CCollections_CIVectorChangedEventArgs * This,
        /* [retval, out] */__RPC__out __x_ABI_CWindows_CFoundation_CCollections_CCollectionChange * value
        );

    /* [propget] */HRESULT ( STDMETHODCALLTYPE *get_Index )(
        __x_ABI_CWindows_CFoundation_CCollections_CIVectorChangedEventArgs * This,
        /* [retval, out] */__RPC__out unsigned int * value
        );

    END_INTERFACE
} __x_ABI_CWindows_CFoundation_CCollections_CIVectorChangedEventArgsVtbl;

interface __x_ABI_CWindows_CFoundation_CCollections_CIVectorChangedEventArgs
{
    CONST_VTBL struct __x_ABI_CWindows_CFoundation_CCollections_CIVectorChangedEventArgsVtbl *lpVtbl;
};
#endif