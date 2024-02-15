#define DWRITE_CORE 1
#include "dwrite_3.h"

/// <summary>
/// Creates a factory object that is used for subsequent creation of individual DWriteCore objects.
/// </summary>
/// <param name="factoryType">Identifies whether the factory object will be shared or isolated.</param>
/// <param name="iid">Identifies the DirectWrite factory interface, such as UUIDOF(IDWriteFactory).</param>
/// <param name="factory">Receives the DirectWrite factory object.</param>
/// <returns>
/// Standard HRESULT error code.
/// </returns>
/// <remarks>
/// This is functionally the same as the DWriteCreateFactory function exported by the system version
/// of DirectWrite. The DWriteCore function has a different name to avoid ambiguity.
/// </remarks>
EXTERN_C HRESULT DWRITE_EXPORT DWriteCoreCreateFactory(
    _In_ DWRITE_FACTORY_TYPE factoryType,
    _In_ REFIID iid,
    _COM_Outptr_ IUnknown** factory
);

/// <summary>
/// Registers an object that receives internal DWriteCore events.
/// </summary>
/// <param name="eventSink">Event sink object to register.</param>
/// <returns>Standard HRESULT error code.</returns>
/// <remarks>This method does NOT add a reference to the event sink object.
/// The caller is expected to hold a reference to the event sink object until
/// it later unregisters it by calling DWriteUnregisterEventSink.</remarks>
EXTERN_C HRESULT DWRITE_EXPORT DWriteCoreRegisterEventSink(
    IDWriteEventSink* eventSink
);

/// <summary>
/// Unregisters an event sink object that was previously registered
/// using DWriteCoreRegisterEventSink.
/// </summary>
/// <param name="eventSink">Event sink object to unregister.</param>
/// </remarks>Since DWriteCore does not hold a reference to the event sink, this 
/// call can be made by the event sink object's destructor.</remarks>
EXTERN_C void DWRITE_EXPORT DWriteCoreUnregisterEventSink(
    IDWriteEventSink* eventSink
);

