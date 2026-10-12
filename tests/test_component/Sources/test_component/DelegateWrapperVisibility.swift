@_spi(WinRTInternal) import UWP
@_spi(WinRTInternal) import WindowsFoundation

// Keep a cross-module compile check for delegate wrapper aliases that are
// referenced by generated projections in dependent Swift modules.
private typealias _UWPDelegateWrapperVisibilityCheck =
    UWP.__ABI_Windows_Storage.StreamedFileDataRequestedHandlerWrapper
private typealias _FoundationDelegateWrapperVisibilityCheck =
    WindowsFoundation.__ABI_Windows_Foundation.AsyncActionCompletedHandlerWrapper
