// Works around limitations of Swift's C++ Interop when exposing the Windows.h GUID type to Swift.
#pragma once

typedef struct GUID_Workaround {
    unsigned long Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char Data4[8];
} GUID_Workaround;

typedef GUID_Workaround CLSID_Workaround;
typedef GUID_Workaround IID_Workaround;
typedef GUID_Workaround UUID_Workaround;

// Functions depending on workaround types
typedef struct IUnknown_Workaround IUnknown_Workaround;
typedef struct IInspectable_Workaround IInspectable_Workaround;
typedef struct IMarshal_Workaround IMarshal_Workaround;

#include <combaseapi.h>
inline HRESULT CoCreateInstance_Workaround(
        const CLSID_Workaround* rclsid,
        LPUNKNOWN pUnkOuter,
        DWORD dwClsContext,
        const IID_Workaround* riid,
        LPVOID* ppv) {
#ifdef __cplusplus
    return CoCreateInstance(*(CLSID*)rclsid, pUnkOuter, dwClsContext, *(IID*)riid, ppv);
#else
    return CoCreateInstance((REFCLSID)rclsid, pUnkOuter, dwClsContext, (REFIID)riid, ppv);
#endif
}

inline HRESULT CoCreateFreeThreadedMarshaler_Workaround(IUnknown_Workaround* pUnkOuter, IUnknown_Workaround** ppunkMarshal) {
    return CoCreateFreeThreadedMarshaler((IUnknown*)pUnkOuter, (IUnknown**)ppunkMarshal);
}

inline int StringFromGUID2_Workaround(const GUID_Workaround* rguid, LPOLESTR lpsz, int cchMax) {
#ifdef __cplusplus
    return StringFromGUID2(*(GUID*)rguid, lpsz, cchMax);
#else
    return StringFromGUID2((REFGUID)rguid, lpsz, cchMax);
#endif
}

#include <rpc.h>
inline RPC_STATUS UuidFromStringA_Workaround(RPC_CSTR StringUuid, UUID_Workaround* Uuid) {
    return UuidFromStringA(StringUuid, (UUID*)Uuid);
}

#include <roapi.h>
inline HRESULT RoActivateInstance_Workaround(
        HSTRING activatableClassId,
        IInspectable_Workaround** instance) {
    return RoActivateInstance(activatableClassId, (IInspectable**)instance);
}

inline HRESULT RoGetActivationFactory_Workaround(
        HSTRING activatableClassId,
        const IID_Workaround* iid,
        void** factory) {
#ifdef __cplusplus
    return RoGetActivationFactory(activatableClassId, *(IID*)iid, factory);
#else
    return RoGetActivationFactory(activatableClassId, (REFIID)iid, factory);
#endif
}

//-------------------------------------------------------------------------------------------------
// Begin the great lie
// Everything below will unknowingly use Xxx_Workaround.
// For example, uses of GUID will be redirected to GUID_Workaround.
//-------------------------------------------------------------------------------------------------
// Preemptively include headers before swapping out the IID type
// This no-op the #includes in generated code, so that it instead relies of our workaround types
#include <EventToken.h>
#include <inspectable.h>
#include <windowscontracts.h>

#define GUID GUID_Workaround
#undef REFGUID // REFGUID is a #define, not a typedef
#define REFGUID const GUID* __MIDL_CONST
#define IID IID_Workaround
#undef REFIID // REFIID is a #define, not a typedef
#define REFIID const IID* __MIDL_CONST
#define CLSID GUID

#define IUnknown IUnknown_Workaround
#define IUnknownVtbl IUnknownVtbl_Workaround
#define IInspectable IInspectable_Workaround
#define IInspectableVtbl IInspectableVtbl_Workaround
#define IMarshal IMarshal_Workaround
#define IMarshalVtbl IMarshalVtbl_Workaround

// iunknown.h
typedef struct IUnknownVtbl
{
    HRESULT ( STDMETHODCALLTYPE *QueryInterface )(
        __RPC__in IUnknown * This,
        /* [in] */ __RPC__in REFIID riid,
        /* [annotation][iid_is][out] */
        _COM_Outptr_  void **ppvObject);

    ULONG ( STDMETHODCALLTYPE *AddRef )(
        __RPC__in IUnknown * This);

    ULONG ( STDMETHODCALLTYPE *Release )(
        __RPC__in IUnknown * This);
} IUnknownVtbl;

interface IUnknown
{
    CONST_VTBL struct IUnknownVtbl *lpVtbl;
};

// IInspectable.h
typedef struct IInspectableVtbl
{
    BEGIN_INTERFACE

    HRESULT ( STDMETHODCALLTYPE *QueryInterface )(
        __RPC__in IInspectable * This,
        /* [in] */ __RPC__in REFIID riid,
        /* [annotation][iid_is][out] */
        _COM_Outptr_  void **ppvObject);

    ULONG ( STDMETHODCALLTYPE *AddRef )(
        __RPC__in IInspectable * This);

    ULONG ( STDMETHODCALLTYPE *Release )(
        __RPC__in IInspectable * This);

    HRESULT ( STDMETHODCALLTYPE *GetIids )(
        __RPC__in IInspectable * This,
        /* [out] */ __RPC__out ULONG *iidCount,
        /* [size_is][size_is][out] */ __RPC__deref_out_ecount_full_opt(*iidCount) IID **iids);

    HRESULT ( STDMETHODCALLTYPE *GetRuntimeClassName )(
        __RPC__in IInspectable * This,
        /* [out] */ __RPC__deref_out_opt HSTRING *className);

    HRESULT ( STDMETHODCALLTYPE *GetTrustLevel )(
        __RPC__in IInspectable * This,
        /* [out] */ __RPC__out TrustLevel *trustLevel);

    END_INTERFACE
} IInspectableVtbl;

interface IInspectable
{
    CONST_VTBL struct IInspectableVtbl *lpVtbl;
};

typedef struct IMarshalVtbl
{
    HRESULT ( STDMETHODCALLTYPE *QueryInterface )(
        IMarshal * This,
        /* [in] */ REFIID riid,
        /* [annotation][iid_is][out] */
        _COM_Outptr_  void **ppvObject);

    ULONG ( STDMETHODCALLTYPE *AddRef )(
        IMarshal * This);

    ULONG ( STDMETHODCALLTYPE *Release )(
        IMarshal * This);

    HRESULT ( STDMETHODCALLTYPE *GetUnmarshalClass )(
        IMarshal * This,
        /* [annotation][in] */
        _In_  REFIID riid,
        /* [annotation][unique][in] */
        _In_opt_  void *pv,
        /* [annotation][in] */
        _In_  DWORD dwDestContext,
        /* [annotation][unique][in] */
        _Reserved_  void *pvDestContext,
        /* [annotation][in] */
        _In_  DWORD mshlflags,
        /* [annotation][out] */
        _Out_  CLSID *pCid);

    HRESULT ( STDMETHODCALLTYPE *GetMarshalSizeMax )(
        IMarshal * This,
        /* [annotation][in] */
        _In_  REFIID riid,
        /* [annotation][unique][in] */
        _In_opt_  void *pv,
        /* [annotation][in] */
        _In_  DWORD dwDestContext,
        /* [annotation][unique][in] */
        _Reserved_  void *pvDestContext,
        /* [annotation][in] */
        _In_  DWORD mshlflags,
        /* [annotation][out] */
        _Out_  DWORD *pSize);

    HRESULT ( STDMETHODCALLTYPE *MarshalInterface )(
        IMarshal * This,
        /* [annotation][unique][in] */
        _In_  IStream *pStm,
        /* [annotation][in] */
        _In_  REFIID riid,
        /* [annotation][unique][in] */
        _In_opt_  void *pv,
        /* [annotation][in] */
        _In_  DWORD dwDestContext,
        /* [annotation][unique][in] */
        _Reserved_  void *pvDestContext,
        /* [annotation][in] */
        _In_  DWORD mshlflags);

    HRESULT ( STDMETHODCALLTYPE *UnmarshalInterface )(
        IMarshal * This,
        /* [annotation][unique][in] */
        _In_  IStream *pStm,
        /* [annotation][in] */
        _In_  REFIID riid,
        /* [annotation][out] */
        _Outptr_  void **ppv);

    HRESULT ( STDMETHODCALLTYPE *ReleaseMarshalData )(
        IMarshal * This,
        /* [annotation][unique][in] */
        _In_  IStream *pStm);

    HRESULT ( STDMETHODCALLTYPE *DisconnectObject )(
        IMarshal * This,
        /* [annotation][in] */
        _In_  DWORD dwReserved);

} IMarshalVtbl;

interface IMarshal
{
    CONST_VTBL struct IMarshalVtbl *lpVtbl;
};