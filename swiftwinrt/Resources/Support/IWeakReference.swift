import C_BINDINGS_MODULE

internal let IID_IWeakReference: IID = IID(Data1: 0x00000037, Data2: 0x0000, Data3: 0x0000, Data4: (0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46)) // 00000037-0000-0000-C000-000000000046

fileprivate extension IUnknownRef {
    func copyTo(_ riid: REFIID?, _ ppvObj: UnsafeMutablePointer<LPVOID?>?) -> HRESULT {
        self.borrow.pointee.lpVtbl.pointee.QueryInterface(self.borrow, riid, ppvObj)
    }
}

func makeWeakReference(_ object: AnyObject, _ result: UnsafeMutablePointer<UnsafeMutableRawPointer?>) throws {
    let wrapper = WeakReferenceWrapper(WeakReference(object))
    try wrapper.toABI {
        _ = $0.pointee.lpVtbl.pointee.AddRef($0)
        result.pointee = UnsafeMutableRawPointer($0)
    }
}

fileprivate class WeakReference {
    public weak var target: AnyObject?
    public init(_ target: AnyObject) {
        self.target = target
    }
}

fileprivate class WeakReferenceWrapper: WinRTAbiBridgeWrapper<IWeakReferenceBridge> {
    init(_ weakReference: WeakReference){
        super.init(IWeakReferenceBridge.makeAbi(), weakReference)
    }
}

fileprivate enum IWeakReferenceBridge: AbiBridge {
    typealias CABI = C_IWeakReference
    typealias SwiftProjection = WeakReference

    static func makeAbi() -> C_IWeakReference {
        return C_IWeakReference(lpVtbl: &IWeakReferenceVTable)
    }

    static func from(abi: ComPtr<C_IWeakReference>?) -> WeakReference? {
        fatalError("Not needed")
    }
}

fileprivate var IWeakReferenceVTable: C_IWeakReferenceVtbl = .init(
    QueryInterface: { pUnk, riid, ppvObject in
        guard let pUnk, let riid, let ppvObject else { return E_INVALIDARG }
        switch riid.pointee {
        case IID_IWeakReference:
            _ = pUnk.pointee.lpVtbl.pointee.AddRef(pUnk)
            return S_OK
        default:
            guard let obj = WeakReferenceWrapper.tryUnwrapFromBase(raw: pUnk) else { return E_NOINTERFACE }
            fatalError("\(#function): Not implemented: something like obj.QueryInterface")
        }
    },
    AddRef: { WeakReferenceWrapper.addRef($0) },
    Release: { WeakReferenceWrapper.release($0) },
    Resolve: { Resolve($0, $1, $2) }
)

fileprivate func Resolve(
        _ this: UnsafeMutablePointer<C_IWeakReference>?,
        _ iid: UnsafePointer<GUID_Workaround>?,
        _ inspectable: UnsafeMutablePointer<UnsafeMutablePointer<C_IInspectable>?>?) -> HRESULT {
    guard let weakReference = WeakReferenceWrapper.tryUnwrapFrom(abi: ComPtr(this)) else { return E_FAIL }
    guard let iid, let inspectable else { return E_INVALIDARG }
    if let target = weakReference.target {
        _ = target
        _ = iid
        fatalError("\(#function): Not implemented: something like target.QueryInterface")
    }
    else {
        inspectable.pointee = nil
        return S_OK
    }
}
