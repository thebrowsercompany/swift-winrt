import CWinRT

#if true // TODO(WIN-860): Remove workaround once C++ interop issues with WinSDK.GUID are fixed.
public typealias GUID = CWinRT.GUID_Workaround
public typealias IID = CWinRT.IID_Workaround
public typealias CLSID = CWinRT.CLSID_Workaround
public typealias REFIID = UnsafePointer<CWinRT.IID_Workaround>
public typealias C_IUnknown = CWinRT.IUnknown_Workaround
public typealias C_IInspectable = CWinRT.IInspectable_Workaround
public typealias C_IInspectableVtbl = CWinRT.IInspectableVtbl_Workaround
public typealias C_IMarshal = CWinRT.IMarshal_Workaround
public typealias C_IMarshalVtbl = CWinRT.IMarshalVtbl_Workaround
public typealias C_IActivationFactory = CWinRT.IActivationFactory_Workaround
public typealias C_IActivationFactoryVtbl = CWinRT.IActivationFactoryVtbl_Workaround
public typealias C_IWeakReference = CWinRT.IWeakReference_Workaround
public typealias C_IWeakReferenceVtbl = CWinRT.IWeakReferenceVtbl_Workaround
public typealias C_IWeakReferenceSource = CWinRT.IWeakReferenceSource_Workaround
public typealias C_IWeakReferenceSourceVtbl = CWinRT.IWeakReferenceSourceVtbl_Workaround
internal let CoCreateInstance = CWinRT.CoCreateInstance_Workaround
internal let UuidFromStringA = CWinRT.UuidFromStringA_Workaround
internal let RoActivateInstance = CWinRT.RoActivateInstance_Workaround
internal let RoGetActivationFactory = CWinRT.RoGetActivationFactory_Workaround
internal let StringFromGUID2 = CWinRT.StringFromGUID2_Workaround
internal let CoCreateFreeThreadedMarshaler = CWinRT.CoCreateFreeThreadedMarshaler_Workaround
#else
public typealias GUID = CWinRT.GUID
public typealias IID = CWinRT.IID
public typealias CLSID = CWinRT.CLSID
public typealias REFIID = UnsafePointer<CWinRT.IID>
public typealias C_IUnknown = CWinRT.IUnknown
public typealias C_IInspectable = CWinRT.IInspectable
public typealias C_IInspectableVtbl = CWinRT.IInspectableVtbl
public typealias C_IMarshal = CWinRT.IMarshal
public typealias C_IMarshalVtbl = CWinRT.IMarshalVtbl
public typealias C_IActivationFactory = CWinRT.IActivationFactory
public typealias C_IActivationFactoryVtbl = CWinRT.IActivationFactoryVtbl
public typealias C_IWeakReference = CWinRT.IWeakReference
public typealias C_IWeakReferenceVtbl = CWinRT.IWeakReferenceVtbl
public typealias C_IWeakReferenceSource = CWinRT.IWeakReferenceSource
public typealias C_IWeakReferenceSourceVtbl = CWinRT.IWeakReferenceSourceVtbl
internal let CoCreateInstance = CWinRT.CoCreateInstance
internal let UuidFromStringA = CWinRT.UuidFromStringA
internal let RoActivateInstance = CWinRT.RoActivateInstance
internal let RoGetActivationFactory = CWinRT.RoGetActivationFactory
internal let StringFromGUID2 = CWinRT.StringFromGUID2
internal let CoCreateFreeThreadedMarshaler = CWinRT.CoCreateFreeThreadedMarshaler
#endif
