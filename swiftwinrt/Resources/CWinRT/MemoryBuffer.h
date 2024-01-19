// The real MemoryBuffer.h will provide different definitions in C vs C++ compilation mode.
// Override it with ours so we ensure we get the C definitions.

// No pragma once, we might include this twice (once for the regular definitions, once for the workaround)

#include <objidl.h>

struct IMemoryBufferByteAccess :
    public IUnknown
{
    // An IMemoryBuffer object is created by a client, and the buffer is provided by IBufferByteAccess::GetBuffer.
    // When IMemoryBufferReference::Close() is called, the code that is using this buffer should set "value" to nullptr,
    // effectively "forgetting" the pointer ot the buffer.
    STDMETHOD(GetBuffer)(_Outptr_result_buffer_(*capacity) BYTE** value, _Out_ UINT32* capacity) = 0;
};
