import C_BINDINGS_MODULE

#if true // TODO(WIN-860): Remove workaround once C++ interop issues with WinSDK.GUID are fixed.
public typealias WCHAR = C_BINDINGS_MODULE.WCHAR_Workaround
public typealias BSTR = C_BINDINGS_MODULE.BSTR_Workaround
public typealias PCWSTR = C_BINDINGS_MODULE.PCWSTR_Workaround?
public typealias GUID = C_BINDINGS_MODULE.GUID_Workaround
public typealias IID = C_BINDINGS_MODULE.IID_Workaround
public typealias CLSID = C_BINDINGS_MODULE.CLSID_Workaround
public typealias REFIID = UnsafePointer<C_BINDINGS_MODULE.IID_Workaround>
public typealias C_IUnknown = C_BINDINGS_MODULE.IUnknown_Workaround
public typealias C_IInspectable = C_BINDINGS_MODULE.IInspectable_Workaround
public typealias C_IInspectableVtbl = C_BINDINGS_MODULE.IInspectableVtbl_Workaround
internal typealias IRestrictedErrorInfo = C_BINDINGS_MODULE.IRestrictedErrorInfo_Workaround
internal let CoCreateInstance = C_BINDINGS_MODULE.CoCreateInstance_Workaround
internal let GetRestrictedErrorInfo = C_BINDINGS_MODULE.GetRestrictedErrorInfo_Workaround
internal let FormatMessageW = C_BINDINGS_MODULE.FormatMessageW_Workaround
internal let UuidFromStringA = C_BINDINGS_MODULE.UuidFromStringA_Workaround
internal let RoActivateInstance = C_BINDINGS_MODULE.RoActivateInstance_Workaround
internal let RoGetActivationFactory = C_BINDINGS_MODULE.RoGetActivationFactory_Workaround
internal let StringFromGUID2 = C_BINDINGS_MODULE.StringFromGUID2_Workaround
internal let SysAllocString = C_BINDINGS_MODULE.SysAllocString_Workaround
internal let SysFreeString = C_BINDINGS_MODULE.SysFreeString_Workaround
internal let SysStringLen = C_BINDINGS_MODULE.SysStringLen_Workaround
internal let WindowsCreateString = C_BINDINGS_MODULE.WindowsCreateString_Workaround
internal let WindowsGetStringRawBuffer = C_BINDINGS_MODULE.WindowsGetStringRawBuffer_Workaround
#else
public typealias WCHAR = C_BINDINGS_MODULE.WCHAR
public typealias BSTR = C_BINDINGS_MODULE.BSTR
public typealias PCWSTR = C_BINDINGS_MODULE.PCWSTR?
public typealias GUID = C_BINDINGS_MODULE.GUID
public typealias IID = C_BINDINGS_MODULE.IID
public typealias CLSID = C_BINDINGS_MODULE.CLSID
public typealias REFIID = UnsafePointer<C_BINDINGS_MODULE.IID>
public typealias C_IUnknown = C_BINDINGS_MODULE.IUnknown
public typealias C_IInspectable = C_BINDINGS_MODULE.IInspectable
public typealias C_IInspectableVtbl = C_BINDINGS_MODULE.IInspectableVtbl
internal typealias IRestrictedErrorInfo = C_BINDINGS_MODULE.IRestrictedErrorInfo
internal let CoCreateInstance = C_BINDINGS_MODULE.CoCreateInstance
internal let GetRestrictedErrorInfo = C_BINDINGS_MODULE.GetRestrictedErrorInfo
internal let FormatMessageW = C_BINDINGS_MODULE.FormatMessageW
internal let UuidFromStringA = C_BINDINGS_MODULE.UuidFromStringA
internal let RoActivateInstance = C_BINDINGS_MODULE.RoActivateInstance
internal let RoGetActivationFactory = C_BINDINGS_MODULE.RoGetActivationFactory
internal let StringFromGUID2 = C_BINDINGS_MODULE.StringFromGUID2
internal let SysAllocString = C_BINDINGS_MODULE.SysAllocString
internal let SysFreeString = C_BINDINGS_MODULE.SysFreeString
internal let SysStringLen = C_BINDINGS_MODULE.SysStringLen
internal let WindowsCreateString = C_BINDINGS_MODULE.WindowsCreateString
internal let WindowsGetStringRawBuffer = C_BINDINGS_MODULE.WindowsGetStringRawBuffer
#endif
