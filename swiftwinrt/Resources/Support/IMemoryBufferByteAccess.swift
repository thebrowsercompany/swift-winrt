import C_BINDINGS_MODULE
import Foundation

public protocol IMemoryBufferByteAccess: IWinRTObject {
    var data: Data { get }
}

extension IMemoryBufferByteAccess {
    public var data: Data {
        var buffer: UnsafeMutablePointer<UInt8>?
        try! perform(as: IMemoryBufferByteAccessABI.self) { pThis in
            try CHECKED(pThis.pointee.lpVtbl.pointee.Buffer(pThis, &buffer))
        }
        return Data(bytesNoCopy: buffer!, count: buffer!.pointee.count, deallocator: .none)
    }
}

fileprivate let IID_IMemoryBufferByteAccess = IID(Data1: 0x5B0D3235 Data2: 0x4DBA, Data3: 0x4D44, Data4: ( 0x86, 0x5E, 0x8F, 0x1D, 0x0E, 0x4F, 0xD0, 0x4D )) // 5b0d3235-4dba-4d44-865e-8f1d0e4fd04d

fileprivate class IMemoryBufferByteAccessABI: IUnknown {
    override class var IID: IID { IID_IMemoryBufferByteAccess}

    func Buffer(_ bytes: UnsafeMutablePointer<UInt8>?) throws {
        var buffer: UnsafeMutablePointer<UInt8>?
        try perform(as: C_BINDINGS_MODULE.IMemoryBufferByteAccess.self) { pThis in
            try CHECKED(pThis.pointee.lpVtbl.pointee.Buffer(pThis, &buffer))
        }
    }

    static func Buffer(_ this: UnsafeMutablePointer<C_BINDINGS_MODULE.IMemoryBufferByteAccess>?, _ buffer: UnsafeMutablePointer<UnsafeMutablePointer<UInt8>?>?) -> HRESULT {
        return E_NOTIMPL
    }
}

fileprivate enum IMemoryBufferByteAccessBridge: ABIBridge {
    static func makeAbi() -> C_IMemoryBufferByteAccess {
        return C_IMemoryBufferByteAccess(lpVtbl: &IMemoryBufferByteAccessVTable)
    }

    static func from(abi: ComPtr<C_IMemoryBufferByteAccess>?) -> Marshaler? {
        guard let abi = abi else { return nil }
        return try? Marshaler(IUnknownRef(abi))
    }

    typealias CABI = C_IMemoryBufferByteAccess
    typealias SwiftProjection = Marshaler
}

fileprivate class MemoryBufferWrapper: WinRTAbiBridgeWrapper<IMemoryBufferByteAccessBridge> {
    init(_ marshaler: Marshaler){
        super.init(IMemoryBufferByteAccessBridge.makeAbi(), marshaler)
    }
}

fileprivate enum IMemoryBufferByteAccessBridge: AbiBridge {

}

private var IMemoryBufferByteAccessVTable: IMemoryBufferByteAccessVtbl = .init(
    QueryInterface: { MemoryBufferWrapper.queryInterface($0, $1, $2) },
    AddRef: { MemoryBufferWrapper.addRef($0) },
    Release: { MemoryBufferWrapper.release($0) },
    Buffer: { IMemoryBufferByteAccessABI.Buffer($0, $1) },
)