// The real robuffer.h doesn't provide C definitions, so we need to provide those here

// No pragma once, we might include this twice (once for the regular definitions, once for the workaround)

#include <apiset.h>
#include <apisetcconv.h>
#include <objidl.h>

#ifndef _IBufferByteAccess_DEFINED
#define _IBufferByteAccess_DEFINED
typedef interface C_IBufferByteAccess C_IBufferByteAccess;
#endif /* _IBufferByteAccess_DEFINED */

typedef struct C_IBufferByteAccessVtbl
{
    BEGIN_INTERFACE

    HRESULT (STDMETHODCALLTYPE* QueryInterface)(__RPC__in C_IBufferByteAccess* This,
        REFIID riid,
        _COM_Outptr_ void** ppvObject);
    ULONG (STDMETHODCALLTYPE* AddRef)(__RPC__in C_IBufferByteAccess* This);
    ULONG (STDMETHODCALLTYPE* Release)(__RPC__in C_IBufferByteAccess* This);
    HRESULT (STDMETHODCALLTYPE* Buffer)(__RPC__in C_IBufferByteAccess* This,
        BYTE** value);

    END_INTERFACE
} C_IBufferByteAccessVtbl;

interface C_IBufferByteAccess
{
    CONST_VTBL struct C_IBufferByteAccessVtbl* lpVtbl;
};
