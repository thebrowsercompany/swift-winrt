import Ctest_component

#if true // TODO(WIN-860): Remove workaround once C++ interop issues with WinSDK.GUID are fixed.
public typealias WCHAR = Ctest_component.WCHAR_Workaround
public typealias PCWSTR = Ctest_component.PCWSTR_Workaround?
public typealias GUID = Ctest_component.GUID_Workaround
public typealias IID = Ctest_component.IID_Workaround
public typealias CLSID = Ctest_component.CLSID_Workaround
public typealias REFIID = UnsafePointer<Ctest_component.IID_Workaround>
public typealias C_IUnknown = Ctest_component.IUnknown_Workaround
public typealias C_IInspectable = Ctest_component.IInspectable_Workaround
public typealias C_IInspectableVtbl = Ctest_component.IInspectableVtbl_Workaround
internal typealias IRestrictedErrorInfo = Ctest_component.IRestrictedErrorInfo_Workaround
internal let CoCreateInstance = Ctest_component.CoCreateInstance_Workaround
internal let GetRestrictedErrorInfo = Ctest_component.GetRestrictedErrorInfo_Workaround
internal let FormatMessageW = Ctest_component.FormatMessageW_Workaround
internal let UuidFromStringA = Ctest_component.UuidFromStringA_Workaround
internal let RoActivateInstance = Ctest_component.RoActivateInstance_Workaround
internal let RoGetActivationFactory = Ctest_component.RoGetActivationFactory_Workaround
internal let StringFromGUID2 = Ctest_component.StringFromGUID2_Workaround
// internal let SysAllocString = Ctest_component.SysAllocString_Workaround
// internal let SysFreeString = Ctest_component.SysFreeString_Workaround
// internal let SysStringLen = Ctest_component.SysStringLen_Workaround
internal let WindowsCreateString = Ctest_component.WindowsCreateString_Workaround
internal let WindowsGetStringRawBuffer = Ctest_component.WindowsGetStringRawBuffer_Workaround
#else
public typealias WCHAR = Ctest_component.WCHAR
public typealias PCWSTR = Ctest_component.PCWSTR?
public typealias GUID = Ctest_component.GUID
public typealias IID = Ctest_component.IID
public typealias CLSID = Ctest_component.CLSID
public typealias REFIID = UnsafePointer<Ctest_component.IID>
public typealias C_IUnknown = Ctest_component.IUnknown
public typealias C_IInspectable = Ctest_component.IInspectable
public typealias C_IInspectableVtbl = Ctest_component.IInspectableVtbl
internal typealias IRestrictedErrorInfo = Ctest_component.IRestrictedErrorInfo
internal let CoCreateInstance = Ctest_component.CoCreateInstance
internal let GetRestrictedErrorInfo = Ctest_component.GetRestrictedErrorInfo
internal let FormatMessageW = Ctest_component.FormatMessageW
internal let UuidFromStringA = Ctest_component.UuidFromStringA
internal let RoActivateInstance = Ctest_component.RoActivateInstance
internal let RoGetActivationFactory = Ctest_component.RoGetActivationFactory
internal let StringFromGUID2 = Ctest_component.StringFromGUID2
// internal let SysAllocString = Ctest_component.SysAllocString
// internal let SysFreeString = Ctest_component.SysFreeString
// internal let SysStringLen = Ctest_component.SysStringLen
internal let WindowsCreateString = Ctest_component.WindowsCreateString
internal let WindowsGetStringRawBuffer = Ctest_component.WindowsGetStringRawBuffer
#endif
