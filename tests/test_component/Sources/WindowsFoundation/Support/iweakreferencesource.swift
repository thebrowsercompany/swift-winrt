import CWinRT

internal let IID_IWeakReferenceSource: IID = IID(Data1: 0x00000038, Data2: 0x0000, Data3: 0x0000, Data4: (0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46)) // 00000038-0000-0000-C000-000000000046

fileprivate extension IUnknownRef {
    func copyTo(_ riid: REFIID?, _ ppvObj: UnsafeMutablePointer<LPVOID?>?) -> HRESULT {
        self.borrow.pointee.lpVtbl.pointee.QueryInterface(self.borrow, riid, ppvObj)
    }
}

func makeWeakReferenceSource(_ object: AnyObject, _ result: UnsafeMutablePointer<UnsafeMutableRawPointer?>) throws {
    let wrapper = WeakReferenceSourceWrapper(object)
    try wrapper.toABI {
        _ = $0.pointee.lpVtbl.pointee.AddRef($0)
        result.pointee = UnsafeMutableRawPointer($0)
    }
}

fileprivate class WeakReferenceSourceWrapper: WinRTAbiBridgeWrapper<IWeakReferenceSourceBridge> {
    init(_ object: AnyObject){
        super.init(IWeakReferenceSourceBridge.makeAbi(), object)
    }

    internal static func queryInterface(_ pUnk: UnsafeMutablePointer<C_IInspectable>?, _ riid: UnsafePointer<WindowsFoundation.IID>?, _ ppvObject: UnsafeMutablePointer<UnsafeMutableRawPointer?>?) -> HRESULT {
        guard let pUnk, let riid, let ppvObject else { return E_INVALIDARG }
        switch riid.pointee {
        case IID_IWeakReferenceSource:
            _ = pUnk.pointee.lpVtbl.pointee.AddRef(pUnk)
            return S_OK
        default:
            guard let obj = WeakReferenceSourceWrapper.tryUnwrapFromBase(raw: pUnk) else { return E_NOINTERFACE }
            let anyWrapper = __ABI_.AnyWrapper(obj)!
            return __ABI_.AnyWrapper.queryInterface(try! anyWrapper.toABI { $0 }, riid, ppvObject)
        }
    }
}

fileprivate enum IWeakReferenceSourceBridge: AbiBridge {
    typealias CABI = C_IWeakReferenceSource
    typealias SwiftProjection = AnyObject

    static func makeAbi() -> C_IWeakReferenceSource {
        return C_IWeakReferenceSource(lpVtbl: &IWeakReferenceSourceVTable)
    }

    static func from(abi: ComPtr<C_IWeakReferenceSource>?) -> AnyObject? {
        fatalError("Not needed")
    }
}

fileprivate var IWeakReferenceSourceVTable: C_IWeakReferenceSourceVtbl = .init(
    QueryInterface: { WeakReferenceSourceWrapper.queryInterface($0, $1, $2) },
    AddRef: { WeakReferenceSourceWrapper.addRef($0) },
    Release: { WeakReferenceSourceWrapper.release($0) },
    GetWeakReference: { GetWeakReference($0, $1) }
)

fileprivate func GetWeakReference(
        _ this: UnsafeMutablePointer<C_IWeakReferenceSource>?,
        _ weakReference: UnsafeMutablePointer<UnsafeMutablePointer<C_IWeakReference>?>?) -> HRESULT {
    guard let object = WeakReferenceSourceWrapper.tryUnwrapFrom(abi: ComPtr(this)) else { return E_FAIL }
    guard let weakReference else { return E_INVALIDARG }
    do {
        var rawWeakReference: UnsafeMutableRawPointer? = nil
        try makeWeakReference(object, &rawWeakReference)
        weakReference.pointee = rawWeakReference?.bindMemory(to: C_IWeakReference.self, capacity: 1)
        return S_OK
    } catch {
        return E_FAIL
    }
}
