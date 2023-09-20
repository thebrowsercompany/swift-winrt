// Defines an alternate GUID2 and associated types that avoids using the GUID type
// due to limitations of Swift interop.
#pragma once

#include <windows.h>
#include <roapi.h>

typedef struct GUID2 {
    unsigned long  Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[8];
} GUID2;

typedef GUID2 UUID2;
#define REFGUID2 const GUID2* __MIDL_CONST
typedef GUID2 CLSID2;
#define REFCLSID2 const CLSID2* __MIDL_CONST
typedef GUID2 IID2;
#define REFIID2 const IID2* __MIDL_CONST

// Basic COM interfaces
inline IID2 GetIID2OfIUnknown() {
    return *(IID2*)&IID_IUnknown;
}

interface IUnknownWithIID2Vtbl;

typedef interface IUnknownWithIID2
{
    CONST_VTBL struct IUnknownWithIID2Vtbl *lpVtbl;
} IUnknownWithIID2;

typedef struct IUnknownWithIID2Vtbl
{
    HRESULT ( STDMETHODCALLTYPE *QueryInterface )(
        __RPC__in IUnknownWithIID2 * This,
        /* [in] */ __RPC__in REFIID2 riid,
        /* [annotation][iid_is][out] */
        _COM_Outptr_  void **ppvObject);

    ULONG ( STDMETHODCALLTYPE *AddRef )(
        __RPC__in IUnknownWithIID2 * This);

    ULONG ( STDMETHODCALLTYPE *Release )(
        __RPC__in IUnknownWithIID2 * This);
} IUnknownWithIID2Vtbl;

inline IID2 GetIID2OfIInspectable() {
    return *(IID2*)&IID_IInspectable;
}

interface IInspectableWithIID2Vtbl;

typedef interface IInspectableWithIID2
{
    CONST_VTBL struct IInspectableWithIID2Vtbl *lpVtbl;
} IInspectableWithIID2;

typedef struct IInspectableWithIID2Vtbl
{
    BEGIN_INTERFACE

    HRESULT ( STDMETHODCALLTYPE *QueryInterface )(
        __RPC__in IInspectableWithIID2 * This,
        /* [in] */ __RPC__in REFIID2 riid,
        /* [annotation][iid_is][out] */
        _COM_Outptr_  void **ppvObject);

    ULONG ( STDMETHODCALLTYPE *AddRef )(
        __RPC__in IInspectableWithIID2 * This);

    ULONG ( STDMETHODCALLTYPE *Release )(
        __RPC__in IInspectableWithIID2 * This);

    HRESULT ( STDMETHODCALLTYPE *GetIids )(
        __RPC__in IInspectableWithIID2 * This,
        /* [out] */ __RPC__out ULONG *iidCount,
        /* [size_is][size_is][out] */ __RPC__deref_out_ecount_full_opt(*iidCount) IID2 **iids);

    HRESULT ( STDMETHODCALLTYPE *GetRuntimeClassName )(
        __RPC__in IInspectableWithIID2 * This,
        /* [out] */ __RPC__deref_out_opt HSTRING *className);

    HRESULT ( STDMETHODCALLTYPE *GetTrustLevel )(
        __RPC__in IInspectableWithIID2 * This,
        /* [out] */ __RPC__out TrustLevel *trustLevel);

    END_INTERFACE
} IInspectableWithIID2Vtbl;

// Methods using GUIDs
inline int StringFromGUID22(REFGUID2 rguid, LPOLESTR lpsz, int cchMax) {
    return StringFromGUID2((REFGUID)(void*)rguid, lpsz, cchMax);
}

inline RPC_STATUS Uuid2FromStringA(RPC_CSTR StringUuid, UUID2* Uuid) {
    return UuidFromStringA(StringUuid, (UUID*)(void*)Uuid);
}

inline HRESULT RoGetActivationFactoryWithIID2(
        HSTRING activatableClassId,
        REFIID2 iid,
        void** factory) {
    return RoGetActivationFactory(activatableClassId, (REFIID)(void*)iid, factory);
}

inline HRESULT RoActivateInstanceWithIID2(
        HSTRING activatableClassId,
        IInspectableWithIID2** instance) {
    return RoActivateInstance(activatableClassId, (IInspectable**)instance);
}

inline HRESULT CoCreateInstanceWithIID2(
        REFIID2 rclsid,
        LPUNKNOWN pUnkOuter,
        DWORD dwClsContext,
        REFIID2 riid,
        LPVOID* ppv) {
    return CoCreateInstance((REFCLSID)(void*)rclsid, pUnkOuter, dwClsContext, (REFIID)(void*)riid, ppv);
}