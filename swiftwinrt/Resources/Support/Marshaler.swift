import WinSDK
import C_BINDINGS_MODULE

extension IUnknownRef {
    func copyTo(_ riid: UnsafeMutablePointer<IID_Workaround>?, _ ppvObj: UnsafeMutablePointer<LPVOID?>?) -> HRESULT {
        self.borrow.pointee.lpVtbl.pointee.QueryInterface(self.borrow, riid, ppvObj)
    }
}

var IMarshal_IID: IID = IID(Data1: 0x01000300, Data2: 0x0000, Data3: 0x0000, Data4: (0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46)) // 01000300-0000-0000-C000-00000046

func makeMarshaler(_ outer: IUnknownRef, _ result: UnsafeMutablePointer<UnsafeMutableRawPointer?>) throws {
    let marshaler = try Marshaler(outer)
    let wrapper = MarshalWrapper(marshaler)
    try wrapper.toABI {
        _ = $0.pointee.lpVtbl.pointee.AddRef($0)
        result.pointee = UnsafeMutableRawPointer($0)
    }
}

fileprivate class MarshalWrapper: WinRTWrapperBase2<IMarshalBridge> {
    init(_ marshaler: Marshaler){
        super.init(IMarshalBridge.makeAbi(), marshaler)
    }
}

fileprivate class IMarshalBridge: AbiBridge {
    static func makeAbi() -> IMarshal {
        return IMarshal(lpVtbl: &IMarshalVTable)
    }

    typealias CABI = IMarshal
    typealias SwiftProjection = Marshaler
}
private var IMarshalVTable: IMarshalVtbl = .init(
    QueryInterface: {
        guard let pUnk = $0, let riid = $1, let ppvObject = $2 else { return E_INVALIDARG }
        return riid.withMemoryRebound(to: IID_Workaround.self, capacity: 1) {
            switch $0.pointee {
            case IMarshal_IID:
                _ = pUnk.pointee.lpVtbl.pointee.AddRef(pUnk)
                return S_OK
            default:
                guard let obj = MarshalWrapper.tryUnwrapFrom(raw: pUnk)?.obj else { return E_NOINTERFACE }
                return obj.copyTo(UnsafeMutablePointer(mutating: $0), ppvObject)
            }
        }
    },
    AddRef: { MarshalWrapper.addRef($0) },
    Release: { MarshalWrapper.release($0) },
    GetUnmarshalClass: { Marshaler.GetUnmarshalClass($0, $1, $2, $3, $4, $5, $6) },
    GetMarshalSizeMax: { Marshaler.GetMarshalSizeMax($0, $1, $2, $3, $4, $5, $6) },
    MarshalInterface: { Marshaler.MarshalInterface($0, $1, $2, $3, $4, $5, $6) },
    UnmarshalInterface: { Marshaler.UnmarshalInterface($0, $1, $2, $3)},
    ReleaseMarshalData: { Marshaler.ReleaseMarshalData($0, $1) },
    DisconnectObject: { Marshaler.DisconnectObject($0, $1) }
)

private class Marshaler {
    var obj: IUnknownRef

    init(_ obj: IUnknownRef) throws {
        self.obj = obj
        var marshalerPtr: LPUNKNOWN?
        try CHECKED(CoCreateFreeThreadedMarshaler(nil, &marshalerPtr))
        guard let marshalerPtr else { throw SUPPORT_MODULE.Error(hr: E_FAIL) }
        marshaler = UnsafeMutableRawPointer(marshalerPtr).bindMemory(to: IMarshal.self, capacity: 1)
    }

    deinit {
        _ = marshaler.pointee.lpVtbl.pointee.Release(marshaler)
    }

    var marshaler: UnsafeMutablePointer<IMarshal>

    static func GetUnmarshalClass(
        _ this: UnsafeMutablePointer<IMarshal>?,
        _ riid: UnsafePointer<WinSDK.IID>?,
        _ pv: UnsafeMutableRawPointer?,
        _ dwDestContext: DWORD,
        _ pvDestContext: UnsafeMutableRawPointer?,
        _ mshlflags: DWORD,
        _ pCid: UnsafeMutablePointer<WinSDK.CLSID>?) -> HRESULT {
        guard let marshaler = MarshalWrapper.tryUnwrapFrom(abi: this)?.marshaler else { return E_FAIL }
        return marshaler.pointee.lpVtbl.pointee.GetUnmarshalClass(marshaler, riid, pv, dwDestContext, pvDestContext, mshlflags, pCid)
    }

    static func GetMarshalSizeMax(
        _ this: UnsafeMutablePointer<IMarshal>?,
        _ riid: UnsafePointer<WinSDK.IID>?, _ pv: UnsafeMutableRawPointer?,
        _ dwDestContext: DWORD,
        _ pvDestContext: UnsafeMutableRawPointer?,
        _ mshlflags: DWORD,
        _ pSize: UnsafeMutablePointer<DWORD>?) -> HRESULT {
        guard let marshaler = MarshalWrapper.tryUnwrapFrom(abi: this)?.marshaler else { return E_FAIL }
        return marshaler.pointee.lpVtbl.pointee.GetMarshalSizeMax(marshaler, riid, pv, dwDestContext, pvDestContext, mshlflags, pSize)
    }

    static func MarshalInterface(
        _ this: UnsafeMutablePointer<IMarshal>?,
        _ pStm: UnsafeMutablePointer<IStream>?,
        _ riid: UnsafePointer<WinSDK.IID>?,
        _ pv: UnsafeMutableRawPointer?,
        _ dwDestContext: DWORD,
        _ pvDestContext: UnsafeMutableRawPointer?,
        _ mshlflags: DWORD) -> HRESULT {
        guard let marshaler = MarshalWrapper.tryUnwrapFrom(abi: this)?.marshaler else { return E_FAIL }
        return marshaler.pointee.lpVtbl.pointee.MarshalInterface(marshaler, pStm, riid, pv, dwDestContext, pvDestContext, mshlflags)
    }

    static func UnmarshalInterface(
        _ this: UnsafeMutablePointer<IMarshal>?,
        _ pStm: UnsafeMutablePointer<IStream>?,
        _ riid: UnsafePointer<WinSDK.IID>?,
        _ ppv: UnsafeMutablePointer<UnsafeMutableRawPointer?>?) -> HRESULT {
        guard let marshaler = MarshalWrapper.tryUnwrapFrom(abi: this)?.marshaler else { return E_FAIL }
        return marshaler.pointee.lpVtbl.pointee.UnmarshalInterface(marshaler, pStm, riid, ppv)
    }

    static func ReleaseMarshalData(
        _ this: UnsafeMutablePointer<IMarshal>?,
        _ pStm: UnsafeMutablePointer<IStream>?) -> HRESULT {
        guard let marshaler = MarshalWrapper.tryUnwrapFrom(abi: this)?.marshaler else { return E_FAIL }
        return marshaler.pointee.lpVtbl.pointee.ReleaseMarshalData(marshaler, pStm)
    }

    static func DisconnectObject(
        _ this: UnsafeMutablePointer<IMarshal>?,
        _ dwReserved: DWORD) -> HRESULT {
        guard let marshaler = MarshalWrapper.tryUnwrapFrom(abi: this)?.marshaler else { return E_FAIL }
        return marshaler.pointee.lpVtbl.pointee.DisconnectObject(marshaler, dwReserved)
    }
}
