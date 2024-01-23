import C_BINDINGS_MODULE
import Foundation

public protocol IMemoryBufferByteAccess: WinRTInterface {
    var data: Data { get throws }
}

public typealias AnyIMemoryBufferByteAccess = any IMemoryBufferByteAccess
fileprivate let IID_IMemoryBufferByteAccess = IID(Data1: 0x5B0D3235, Data2: 0x4DBA, Data3: 0x4D44, Data4: ( 0x86, 0x5E, 0x8F, 0x1D, 0x0E, 0x4F, 0xD0, 0x4D )) // 5b0d3235-4dba-4d44-865e-8f1d0e4fd04d

extension __ABI_ {
    public class IMemoryBufferByteAccess: IUnknown {
        override public class var IID: IID { IID_IMemoryBufferByteAccess}

        public func Buffer() throws -> UnsafeMutablePointer<UInt8>? {
            var buffer: UnsafeMutablePointer<UInt8>?
            var capacity: UInt32 = 0
            try perform(as: C_BINDINGS_MODULE.IMemoryBufferByteAccess.self) { pThis in
                try CHECKED(pThis.pointee.lpVtbl.pointee.GetBuffer(pThis, &buffer, &capacity))
            }
            return buffer
        }

        fileprivate static func GetBuffer(_ this: UnsafeMutablePointer<C_BINDINGS_MODULE.IMemoryBufferByteAccess>?, _ buffer: UnsafeMutablePointer<UnsafeMutablePointer<UInt8>?>?, _ count: UnsafeMutablePointer<UInt32>?) -> HRESULT {
            return E_NOTIMPL
        }
    }

    public typealias IMemoryBufferByteAccessWrapper = InterfaceWrapperBase<IMemoryBufferByteAccessBridge>
}

public enum IMemoryBufferByteAccessBridge: AbiInterfaceBridge {
    public static func makeAbi() -> CABI {
        return CABI(lpVtbl: &IMemoryBufferByteAccessVTable)
    }

    public static func from(abi: ComPtr<CABI>?) -> SwiftProjection? {
        guard let abi = abi else { return nil }
        return IMemoryBufferByteAccessImpl(abi)
    }

    public typealias CABI = C_BINDINGS_MODULE.IMemoryBufferByteAccess
    public typealias SwiftABI = __ABI_.IMemoryBufferByteAccess
    public typealias SwiftProjection = AnyIMemoryBufferByteAccess
}

fileprivate class IMemoryBufferByteAccessImpl: AbiInterfaceImpl, IMemoryBufferByteAccess {
    typealias Bridge = IMemoryBufferByteAccessBridge
    let _default: Bridge.SwiftABI

    fileprivate init(_ fromAbi: ComPtr<Bridge.CABI>) {
        _default = Bridge.SwiftABI(fromAbi)
    }

    var data: Data {
        // in order to implement these, we need to QI for IMemoryBufferReference
        fatalError("not implemented")
    }
}

fileprivate var IMemoryBufferByteAccessVTable: C_BINDINGS_MODULE.IMemoryBufferByteAccessVtbl = .init(
    QueryInterface: {  __ABI_.IMemoryBufferByteAccessWrapper.queryInterface($0, $1, $2) },
    AddRef: {  __ABI_.IMemoryBufferByteAccessWrapper.addRef($0) },
    Release: {  __ABI_.IMemoryBufferByteAccessWrapper.release($0) },
    GetBuffer: { __ABI_.IMemoryBufferByteAccess.GetBuffer($0, $1, $2) }
)

extension IMemoryBufferByteAccess {
    public func queryInterface(_ iid: SUPPORT_MODULE.IID) -> IUnknownRef? {
        switch iid {
            case __ABI_.IMemoryBufferByteAccessWrapper.IID:
                let wrapper = __ABI_.IMemoryBufferByteAccessWrapper(self)
                return wrapper!.queryInterface(iid)
            default: return nil
        }
    }
}