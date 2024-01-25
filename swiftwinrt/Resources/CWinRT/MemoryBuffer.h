// The real MemoryBuffer.h will provide different definitions in C vs C++ compilation mode.
// Override it with ours so we ensure we get the C definitions.

// No pragma once, we might include this twice (once for the regular definitions, once for the workaround)

#include <objidl.h>

#ifndef _IMEMORYBUFFERBYTEACCESS_DEFINED
#define _IMEMORYBUFFERBYTEACCESS_DEFINED
typedef interface IMemoryBufferByteAccess IMemoryBufferByteAccess;
#endif

typedef struct IMemoryBufferByteAccessVtbl
{
    BEGIN_INTERFACE

    HRESULT (STDMETHODCALLTYPE* QueryInterface)(__RPC__in IMemoryBufferByteAccess* This,
        REFIID riid,
        _COM_Outptr_ void** ppvObject);
    ULONG (STDMETHODCALLTYPE* AddRef)(__RPC__in IMemoryBufferByteAccess* This);
    ULONG (STDMETHODCALLTYPE* Release)(__RPC__in IMemoryBufferByteAccess* This);
    HRESULT (STDMETHODCALLTYPE* GetBuffer)(__RPC__in IMemoryBufferByteAccess* This,
        BYTE** value,
        _Out_ UINT32* capacity);

    END_INTERFACE
} IMemoryBufferByteAccessVtbl;

interface IMemoryBufferByteAccess
{
    CONST_VTBL struct IMemoryBufferByteAccessVtbl* lpVtbl;
};