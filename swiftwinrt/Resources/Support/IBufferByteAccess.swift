import C_BINDINGS_MODULE
import Foundation

public protocol IBufferByteAccess: WinRTInterface {
    var data: Data { get throws }
}

public typealias AnyIBufferByteAccess = any IBufferByteAccess

fileprivate let IID_IBufferByteAccess = IID(Data1: 0x905A0FEF, Data2: 0xBC53, Data3: 0x11DF, Data4: ( 0x8C, 0x49, 0x00, 0x1E, 0x4F, 0xC6, 0x86, 0xDA )) // 905a0fef-bc53-11df-8c49-001e4fc686da

extension __ABI_ {
    public class IBufferByteAccess: IUnknown {
        override public class var IID: IID { IID_IBufferByteAccess}

        public func Buffer(_ bytes: UnsafeMutablePointer<UInt8>?) throws {
            var buffer: UnsafeMutablePointer<UInt8>?
            try perform(as: C_BINDINGS_MODULE.C_IBufferByteAccess.self) { pThis in
                try CHECKED(pThis.pointee.lpVtbl.pointee.Buffer(pThis, &buffer))
            }
        }
        static fileprivate func Buffer(_ this: UnsafeMutablePointer<C_BINDINGS_MODULE.C_IBufferByteAccess>?, _ buffer: UnsafeMutablePointer<UnsafeMutablePointer<UInt8>?>?) -> HRESULT {
            return E_NOTIMPL
        }
    }

    public typealias IBufferByteAccessWrapper = InterfaceWrapperBase<IBufferByteAccessBridge>
}

public enum IBufferByteAccessBridge: AbiInterfaceBridge {
    public static func makeAbi() -> CABI {
        return CABI(lpVtbl: &IBufferByteAccessVTable)
    }

    public static func from(abi: ComPtr<CABI>?) -> SwiftProjection? {
        guard let abi = abi else { return nil }
        return IBufferByteAccessImpl(abi)
    }

    public typealias CABI = C_BINDINGS_MODULE.C_IBufferByteAccess
    public typealias SwiftABI = __ABI_.IBufferByteAccess
    public typealias SwiftProjection = AnyIBufferByteAccess
}

fileprivate class IBufferByteAccessImpl: AbiInterfaceImpl, IBufferByteAccess {
    typealias Bridge = IBufferByteAccessBridge
    let _default: Bridge.SwiftABI

    fileprivate init(_ fromAbi: ComPtr<Bridge.CABI>) {
        _default = Bridge.SwiftABI(fromAbi)
    }

    var data: Data {
        // in order to implement these, we need to QI for IMemoryBufferReference. in reality
        // this API will never be called because the only way to get an IBufferByteAccess is
        // through IBuffer, and IBufferImpl will have the proper implementation
        fatalError("not implemented")
    }
}

private var IBufferByteAccessVTable: C_BINDINGS_MODULE.C_IBufferByteAccessVtbl = .init(
    QueryInterface: { __ABI_.IBufferByteAccessWrapper.queryInterface($0, $1, $2) },
    AddRef: {  __ABI_.IBufferByteAccessWrapper.addRef($0) },
    Release: {  __ABI_.IBufferByteAccessWrapper.release($0) },
    Buffer: { __ABI_.IBufferByteAccess.Buffer($0, $1) }
)

extension IBufferByteAccess {
    public func queryInterface(_ iid: SUPPORT_MODULE.IID) -> IUnknownRef? {
        switch iid {
            case __ABI_.IBufferByteAccessWrapper.IID:
                let wrapper = __ABI_.IBufferByteAccessWrapper(self)
                return wrapper!.queryInterface(iid)
            default: return nil
        }
    }
}