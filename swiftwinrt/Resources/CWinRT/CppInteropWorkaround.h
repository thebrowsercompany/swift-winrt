// Works around limitations of Swift's C++ Interop
// when exposing wchar_t or the Windows.h GUID type to Swift.
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

typedef UINT16 WCHAR_Workaround;
typedef WCHAR_Workaround OLECHAR_Workaround;
typedef const WCHAR_Workaround* PCWSTR_Workaround;
typedef WCHAR_Workaround* LPWSTR_Workaround;
typedef const WCHAR_Workaround* LPCWSTR_Workaround;
typedef OLECHAR_Workaround* BSTR_Workaround;

// Functions depending on workaround types
typedef struct IUnknown_Workaround IUnknown_Workaround;
typedef struct IInspectable_Workaround IInspectable_Workaround;
typedef struct IRestrictedErrorInfo_Workaround IRestrictedErrorInfo_Workaround;

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

inline int StringFromGUID2_Workaround(const GUID_Workaround* rguid, UINT16* lpsz, int cchMax) {
#ifdef __cplusplus
    return StringFromGUID2(*(GUID*)rguid, (LPOLESTR)lpsz, cchMax);
#else
    return StringFromGUID2((REFGUID)rguid, (LPOLESTR)lpsz, cchMax);
#endif
}

#include <oleauto.h>
inline BSTR_Workaround SysAllocString_Workaround(const OLECHAR_Workaround *psz) {
    // Swift toolchains below 202309?? will hit a build error below
    // because of a bug in the WinSDK modulemap for OLE headers.
    return (BSTR_Workaround)SysAllocString((const OLECHAR*)psz);
}

inline void SysFreeString_Workaround(BSTR_Workaround bstrString) {
    SysFreeString((BSTR)bstrString);
}

inline UINT SysStringLen_Workaround(BSTR_Workaround pbstr) {
    return SysStringLen((BSTR)pbstr);
}

#include <rpc.h>
inline RPC_STATUS UuidFromStringA_Workaround(RPC_CSTR StringUuid, UUID_Workaround* Uuid) {
    return UuidFromStringA(StringUuid, (UUID*)Uuid);
}

#include <roerrorapi.h>
inline HRESULT GetRestrictedErrorInfo_Workaround(IRestrictedErrorInfo_Workaround **ppRestrictedErrorInfo) {
    return GetRestrictedErrorInfo((IRestrictedErrorInfo**)ppRestrictedErrorInfo);
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

#include <windows.h>
inline DWORD FormatMessageW_Workaround(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId, DWORD dwLanguageId, LPWSTR_Workaround lpBuffer, DWORD nSize, va_list *Arguments) {
    return FormatMessageW(dwFlags, lpSource, dwMessageId, dwLanguageId, (LPWSTR)lpBuffer, nSize, Arguments);
}

#include <winstring.h>
inline HRESULT WindowsCreateString_Workaround(const UINT16* sourceString, UINT32 length, HSTRING *string) {
    return WindowsCreateString((LPCWSTR)sourceString, length, string);
}

inline LPCWSTR_Workaround WindowsGetStringRawBuffer_Workaround(HSTRING string, UINT32 *length) {
    return (LPCWSTR_Workaround)WindowsGetStringRawBuffer(string, length);
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

#define WCHAR WCHAR_Workaround
#define OLECHAR WCHAR_Workaround
#define PCWSTR PCWSTR_Workaround
#define LPCWSTR LPCWSTR_Workaround
#define BSTR BSTR_Workaround

#define IUnknown IUnknown_Workaround
#define IUnknownVtbl IUnknownVtbl_Workaround
#define IInspectable IInspectable_Workaround
#define IInspectableVtbl IInspectableVtbl_Workaround
#define IRestrictedErrorInfo IRestrictedErrorInfo_Workaround
#define IRestrictedErrorInfoVtbl IRestrictedErrorInfo_WorkaroundVtbl
#define IID_IRestrictedErrorInfo IID_IRestrictedErrorInfo_Workaround

// Redefine IRestrictedErrorInfo with its workaround name
#undef __IRestrictedErrorInfo_FWD_DEFINED__
#undef __IRestrictedErrorInfo_INTERFACE_DEFINED__
#include "RestrictedErrorInfo.h"

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
