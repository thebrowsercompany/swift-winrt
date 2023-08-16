// This header is written to make handling of IAsyncInfo simpler in the swiftwinrt
// code generation by giving the interface a mangled name

/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 501
#endif

/* verify that the <rpcsal.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCSAL_H_VERSION__
#define __REQUIRED_RPCSAL_H_VERSION__ 100
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif /* __RPCNDR_H_VERSION__ */

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __asyncinfo_h__
#define __asyncinfo_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef DECLSPEC_XFGVIRT
#if defined(_CONTROL_FLOW_GUARD_XFG)
#define DECLSPEC_XFGVIRT(base, func) __declspec(xfg_virtual(base, func))
#else
#define DECLSPEC_XFGVIRT(base, func)
#endif
#endif

/* Forward Declarations */ 

#ifndef ____x_ABI_CWindows_CFoundation_CIAsyncInfo_FWD_DEFINED__
#define ____x_ABI_CWindows_CFoundation_CIAsyncInfo_FWD_DEFINED__
typedef interface __x_ABI_CWindows_CFoundation_CIAsyncInfo __x_ABI_CWindows_CFoundation_CIAsyncInfo;

#endif 	/* ____x_ABI_CWindows_CFoundation_CIAsyncInfo_FWD_DEFINED__ */


/* header files for imported files */
#include "inspectable.h"

/* interface __MIDL_itf_asyncinfo_0000_0000 */
/* [local] */ 

typedef /* [v1_enum] */ 
enum __x_ABI_CWindows_CFoundation_CAsyncStatus
    {
        __x_ABI_CWindows_CFoundation_CAsyncStatus_Started	= 0,
        __x_ABI_CWindows_CFoundation_CAsyncStatus_Completed	= ( __x_ABI_CWindows_CFoundation_CAsyncStatus_Started + 1 ) ,
        __x_ABI_CWindows_CFoundation_CAsyncStatus_Canceled	= ( __x_ABI_CWindows_CFoundation_CAsyncStatus_Completed + 1 ) ,
        __x_ABI_CWindows_CFoundation_CAsyncStatus_Error	= ( __x_ABI_CWindows_CFoundation_CAsyncStatus_Canceled + 1 ) 
    } 	__x_ABI_CWindows_CFoundation_CAsyncStatus;


#ifndef ____x_ABI_CWindows_CFoundation_CIAsyncInfo_INTERFACE_DEFINED__
#define ____x_ABI_CWindows_CFoundation_CIAsyncInfo_INTERFACE_DEFINED__

/* interface __x_ABI_CWindows_CFoundation_CIAsyncInfo */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID___x_ABI_CWindows_CFoundation_CIAsyncInfo;

    typedef struct __x_ABI_CWindows_CFoundation_CIAsyncInfoVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            __RPC__in __x_ABI_CWindows_CFoundation_CIAsyncInfo * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            __RPC__in __x_ABI_CWindows_CFoundation_CIAsyncInfo * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            __RPC__in __x_ABI_CWindows_CFoundation_CIAsyncInfo * This);
        
        DECLSPEC_XFGVIRT(IInspectable, GetIids)
        HRESULT ( STDMETHODCALLTYPE *GetIids )( 
            __RPC__in __x_ABI_CWindows_CFoundation_CIAsyncInfo * This,
            /* [out] */ __RPC__out ULONG *iidCount,
            /* [size_is][size_is][out] */ __RPC__deref_out_ecount_full_opt(*iidCount) IID **iids);
        
        DECLSPEC_XFGVIRT(IInspectable, GetRuntimeClassName)
        HRESULT ( STDMETHODCALLTYPE *GetRuntimeClassName )( 
            __RPC__in __x_ABI_CWindows_CFoundation_CIAsyncInfo * This,
            /* [out] */ __RPC__deref_out_opt HSTRING *className);
        
        DECLSPEC_XFGVIRT(IInspectable, GetTrustLevel)
        HRESULT ( STDMETHODCALLTYPE *GetTrustLevel )( 
            __RPC__in __x_ABI_CWindows_CFoundation_CIAsyncInfo * This,
            /* [out] */ __RPC__out TrustLevel *trustLevel);
        
        DECLSPEC_XFGVIRT(__x_ABI_CWindows_CFoundation_CIAsyncInfo, get_Id)
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE *get_Id )( 
            __RPC__in __x_ABI_CWindows_CFoundation_CIAsyncInfo * This,
            /* [retval][out] */ __RPC__out unsigned __int32 *id);
        
        DECLSPEC_XFGVIRT(__x_ABI_CWindows_CFoundation_CIAsyncInfo, get_Status)
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            __RPC__in __x_ABI_CWindows_CFoundation_CIAsyncInfo * This,
            /* [retval][out] */ __RPC__out __x_ABI_CWindows_CFoundation_CAsyncStatus *status);
        
        DECLSPEC_XFGVIRT(__x_ABI_CWindows_CFoundation_CIAsyncInfo, get_ErrorCode)
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE *get_ErrorCode )( 
            __RPC__in __x_ABI_CWindows_CFoundation_CIAsyncInfo * This,
            /* [retval][out] */ __RPC__out HRESULT *errorCode);
        
        DECLSPEC_XFGVIRT(__x_ABI_CWindows_CFoundation_CIAsyncInfo, Cancel)
        HRESULT ( STDMETHODCALLTYPE *Cancel )( 
            __RPC__in __x_ABI_CWindows_CFoundation_CIAsyncInfo * This);
        
        DECLSPEC_XFGVIRT(__x_ABI_CWindows_CFoundation_CIAsyncInfo, Close)
        HRESULT ( STDMETHODCALLTYPE *Close )( 
            __RPC__in __x_ABI_CWindows_CFoundation_CIAsyncInfo * This);
        
        END_INTERFACE
    } __x_ABI_CWindows_CFoundation_CIAsyncInfoVtbl;

    interface __x_ABI_CWindows_CFoundation_CIAsyncInfo
    {
        CONST_VTBL struct __x_ABI_CWindows_CFoundation_CIAsyncInfoVtbl *lpVtbl;
    };


#endif 	/* ____x_ABI_CWindows_CFoundation_CIAsyncInfo_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_asyncinfo_0000_0001 */
/* [local] */ 

extern RPC_IF_HANDLE __MIDL_itf_asyncinfo_0000_0001_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_asyncinfo_0000_0001_v0_0_s_ifspec;


#endif

