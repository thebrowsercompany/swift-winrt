import C_BINDINGS_MODULE

#if true // TODO(WIN-860): Remove workaround once C++ interop issues with WinSDK.GUID are fixed.
public typealias GUID = C_BINDINGS_MODULE.GUID_Workaround
public typealias IID = C_BINDINGS_MODULE.IID_Workaround
public typealias CLSID = C_BINDINGS_MODULE.CLSID_Workaround
public typealias REFIID = UnsafePointer<C_BINDINGS_MODULE.IID_Workaround>
public typealias C_IUnknown = C_BINDINGS_MODULE.IUnknown_Workaround
public typealias C_IInspectable = C_BINDINGS_MODULE.IInspectable_Workaround
public typealias C_IInspectableVtbl = C_BINDINGS_MODULE.IInspectableVtbl_Workaround
public typealias C_IMarshal = C_BINDINGS_MODULE.IMarshal_Workaround
public typealias C_IMarshalVtbl = C_BINDINGS_MODULE.IMarshalVtbl_Workaround
public typealias C_IActivationFactory = C_BINDINGS_MODULE.IActivationFactory_Workaround
public typealias C_IActivationFactoryVtbl = C_BINDINGS_MODULE.IActivationFactoryVtbl_Workaround
public typealias C_IWeakReference = C_BINDINGS_MODULE.IWeakReference_Workaround
public typealias C_IWeakReferenceVtbl = C_BINDINGS_MODULE.IWeakReferenceVtbl_Workaround
public typealias C_IWeakReferenceSource = C_BINDINGS_MODULE.IWeakReferenceSource_Workaround
public typealias C_IWeakReferenceSourceVtbl = C_BINDINGS_MODULE.IWeakReferenceSourceVtbl_Workaround
internal let CoCreateInstance = C_BINDINGS_MODULE.CoCreateInstance_Workaround
internal let UuidFromStringA = C_BINDINGS_MODULE.UuidFromStringA_Workaround
internal let RoActivateInstance = C_BINDINGS_MODULE.RoActivateInstance_Workaround
internal let RoGetActivationFactory = C_BINDINGS_MODULE.RoGetActivationFactory_Workaround
internal let StringFromGUID2 = C_BINDINGS_MODULE.StringFromGUID2_Workaround
internal let CoCreateFreeThreadedMarshaler = C_BINDINGS_MODULE.CoCreateFreeThreadedMarshaler_Workaround
#else
public typealias GUID = C_BINDINGS_MODULE.GUID
public typealias IID = C_BINDINGS_MODULE.IID
public typealias CLSID = C_BINDINGS_MODULE.CLSID
public typealias REFIID = UnsafePointer<C_BINDINGS_MODULE.IID>
public typealias C_IUnknown = C_BINDINGS_MODULE.IUnknown
public typealias C_IInspectable = C_BINDINGS_MODULE.IInspectable
public typealias C_IInspectableVtbl = C_BINDINGS_MODULE.IInspectableVtbl
public typealias C_IMarshal = C_BINDINGS_MODULE.IMarshal
public typealias C_IMarshalVtbl = C_BINDINGS_MODULE.IMarshalVtbl
public typealias C_IActivationFactory = C_BINDINGS_MODULE.IActivationFactory
public typealias C_IActivationFactoryVtbl = C_BINDINGS_MODULE.IActivationFactoryVtbl
public typealias C_IWeakReference = C_BINDINGS_MODULE.IWeakReference
public typealias C_IWeakReferenceVtbl = C_BINDINGS_MODULE.IWeakReferenceVtbl
public typealias C_IWeakReferenceSource = C_BINDINGS_MODULE.IWeakReferenceSource
public typealias C_IWeakReferenceSourceVtbl = C_BINDINGS_MODULE.IWeakReferenceSourceVtbl
internal let CoCreateInstance = C_BINDINGS_MODULE.CoCreateInstance
internal let UuidFromStringA = C_BINDINGS_MODULE.UuidFromStringA
internal let RoActivateInstance = C_BINDINGS_MODULE.RoActivateInstance
internal let RoGetActivationFactory = C_BINDINGS_MODULE.RoGetActivationFactory
internal let StringFromGUID2 = C_BINDINGS_MODULE.StringFromGUID2
internal let CoCreateFreeThreadedMarshaler = C_BINDINGS_MODULE.CoCreateFreeThreadedMarshaler
#endif
