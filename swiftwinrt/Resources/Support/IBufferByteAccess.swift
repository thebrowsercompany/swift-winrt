import C_BINDINGS_MODULE
import Foundation

/// IBufferByteAccess provides direct access to the underlying bytes of an IBuffer.
/// This buffer is only valid for the lifetime of the IBuffer. For a read-only representation
/// of the buffer, access the bytes through the Data property of the IBuffer.
/// [Open Microsoft Documentation](https://learn.microsoft.com/en-us/windows/win32/api/robuffer/ns-robuffer-ibufferbyteaccess)
public protocol IBufferByteAccess: WinRTInterface {
    var buffer: UnsafeMutablePointer<UInt8>? { get throws }
}

public typealias AnyIBufferByteAccess = any IBufferByteAccess

fileprivate let IID_IBufferByteAccess = IID(Data1: 0x905A0FEF, Data2: 0xBC53, Data3: 0x11DF, Data4: ( 0x8C, 0x49, 0x00, 0x1E, 0x4F, 0xC6, 0x86, 0xDA )) // 905a0fef-bc53-11df-8c49-001e4fc686da

extension __ABI_ {
    public class IBufferByteAccess: IUnknown {
        override public class var IID: IID { IID_IBufferByteAccess}

        public func Buffer() throws -> UnsafeMutablePointer<UInt8>? {
            var buffer: UnsafeMutablePointer<UInt8>?
            try perform(as: C_BINDINGS_MODULE.C_IBufferByteAccess.self) { pThis in
                try CHECKED(pThis.pointee.lpVtbl.pointee.Buffer(pThis, &buffer))
            }
            return buffer
        }
    }

    public typealias IBufferByteAccessWrapper = InterfaceWrapperBase<IBufferByteAccessBridge>
}

public enum IBufferByteAccessBridge: AbiInterfaceBridge {
    public static func makeAbi() -> CABI {
        return CABI(lpVtbl: &IBufferByteAccessVTable)
    }

    public static func from(abi: ComPtr<CABI>?) -> SwiftProjection? {
        // This code path is not actually reachable since IBufferByteAccess is not a WinRT interface.
        // It is a COM interface which is implemented by any object which implements the IBuffer interface.
        // And the IBufferImpl object will correctly have the implementation of this interface, so this isn't needed
        assertionFailure("IBufferByteAccessBridge.from not implemented")
        return nil
    }

    public typealias CABI = C_BINDINGS_MODULE.C_IBufferByteAccess
    public typealias SwiftABI = __ABI_.IBufferByteAccess
    public typealias SwiftProjection = AnyIBufferByteAccess
}

private var IBufferByteAccessVTable: C_BINDINGS_MODULE.C_IBufferByteAccessVtbl = .init(
    QueryInterface: { __ABI_.IBufferByteAccessWrapper.queryInterface($0, $1, $2) },
    AddRef: {  __ABI_.IBufferByteAccessWrapper.addRef($0) },
    Release: {  __ABI_.IBufferByteAccessWrapper.release($0) },
    Buffer: { __ABI_.IBufferByteAccessWrapper.buffer($0, $1) }
)

extension __ABI_.IBufferByteAccessWrapper {
    fileprivate static func buffer(_ this: UnsafeMutablePointer<C_BINDINGS_MODULE.C_IBufferByteAccess>?, _ buffer: UnsafeMutablePointer<UnsafeMutablePointer<UInt8>?>?) -> HRESULT {
        guard let swiftObj = __ABI_.IBufferByteAccessWrapper.tryUnwrapFrom(raw: this) else { return E_INVALIDARG }
        guard let buffer else { return E_INVALIDARG }
        buffer.pointee = try? swiftObj.buffer
        return S_OK
    }
}