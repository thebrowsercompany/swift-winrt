import WinSDK

var IMarshal_IID: IID = IID(Data1: 0x01000300, Data2: 0x0000, Data3: 0x0000, Data4: (0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46)) // 01000300-0000-0000-C000-00000046

func makeMarshaler(_ outer: IUnknownRef, _ result: UnsafeMutablePointer<UnsafeMutableRawPointer?>?) -> HRESULT {
    do {
        let marshaler = try Marshaler(outer)
        let wrapper = MarshalWrapper(marshaler)
        try wrapper.toABI {
            _ = $0.pointee.lpVtbl.pointee.AddRef($0)
            result?.pointee = UnsafeMutableRawPointer($0)
        }
        return S_OK
    } catch {
        if let winrtError = error as? test_component.Error {
            return winrtError.hr
        } else {
            return E_FAIL
        }
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
    QueryInterface: { _, _, _ in return E_NOINTERFACE },
    AddRef: { _ in return 0 },
    Release: { _ in return 0 },
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
        guard let marshalerPtr else { throw test_component.Error(hr: E_FAIL) }
        marshaler = IUnknownRef(consuming: marshalerPtr)
    }

    var marshaler: IUnknownRef

    static func GetUnmarshalClass(_ this: UnsafeMutablePointer<IMarshal>?, _ riid: UnsafePointer<WinSDK.IID>?, _ pv: UnsafeMutableRawPointer?, _ dwDestContext: DWORD, _ pvDestContext: UnsafeMutableRawPointer?, _ mshlflags: DWORD, _ pCid: UnsafeMutablePointer<WinSDK.CLSID>?) -> HRESULT {
        guard let marshaler = MarshalWrapper.tryUnwrapFrom(abi: this)?.marshaler else { return E_FAIL }
        guard letmarshaler.
        return E_NOTIMPL
    }

    static func GetMarshalSizeMax(_ this: UnsafeMutablePointer<IMarshal>?, _ riid: UnsafePointer<WinSDK.IID>?, _ pv: UnsafeMutableRawPointer?, _ dwDestContext: DWORD, _ pvDestContext: UnsafeMutableRawPointer?, _ mshlflags: DWORD, _ pSize: UnsafeMutablePointer<DWORD>?) -> HRESULT {
        guard let marshaler = MarshalWrapper.tryUnwrapFrom(abi: this) else { return E_FAIL }

        return E_NOTIMPL
    }

    static func MarshalInterface(_ this: UnsafeMutablePointer<IMarshal>?, _ pStm: UnsafeMutablePointer<IStream>?, _ riid: UnsafePointer<WinSDK.IID>?, _ pv: UnsafeMutableRawPointer?, _ dwDestContext: DWORD, _ pvDestContext: UnsafeMutableRawPointer?, _ mshlflags: DWORD) -> HRESULT {
        guard let marshaler = MarshalWrapper.tryUnwrapFrom(abi: this) else { return E_FAIL }

        return E_NOTIMPL
    }

    static func UnmarshalInterface(_ this: UnsafeMutablePointer<IMarshal>?, _ pStm: UnsafeMutablePointer<IStream>?, _ riid: UnsafePointer<WinSDK.IID>?, _ ppv: UnsafeMutablePointer<UnsafeMutableRawPointer?>?) -> HRESULT {
        guard let marshaler = MarshalWrapper.tryUnwrapFrom(abi: this) else { return E_FAIL }

        return E_NOTIMPL
    }

    static func ReleaseMarshalData(_ this: UnsafeMutablePointer<IMarshal>?, _ pStm: UnsafeMutablePointer<IStream>?) -> HRESULT {
        guard let marshaler = MarshalWrapper.tryUnwrapFrom(abi: this) else { return E_FAIL }

        return E_NOTIMPL
    }

    static func DisconnectObject(_ this: UnsafeMutablePointer<IMarshal>?, _ dwReserved: DWORD) -> HRESULT {
        guard let marshaler = MarshalWrapper.tryUnwrapFrom(abi: this) else { return E_FAIL }

        return E_NOTIMPL
    }
}