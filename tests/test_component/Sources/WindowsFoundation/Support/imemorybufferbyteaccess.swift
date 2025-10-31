import CWinRT
import Foundation

/// IMemoryBufferByteAccess provides direct access to the underlying bytes of an IMemoryBuffer.
/// This buffer is only valid for the lifetime of the buffer. For a read-only representation
/// of the buffer, access the bytes through the Data property of the IBuffer.
/// [Open Microsoft Documentation](https://learn.microsoft.com/en-us/previous-versions/mt297505(v=vs.85))
public protocol IMemoryBufferByteAccess: WinRTInterface {
    var buffer: UnsafeMutableBufferPointer<UInt8>? { get throws }
}

public typealias AnyIMemoryBufferByteAccess = any IMemoryBufferByteAccess
fileprivate let IID_IMemoryBufferByteAccess = IID(Data1: 0x5B0D3235, Data2: 0x4DBA, Data3: 0x4D44, Data4: ( 0x86, 0x5E, 0x8F, 0x1D, 0x0E, 0x4F, 0xD0, 0x4D )) // 5b0d3235-4dba-4d44-865e-8f1d0e4fd04d

extension __ABI_ {
    public class IMemoryBufferByteAccess: IUnknown {
        override public class var IID: IID { IID_IMemoryBufferByteAccess}

        public func Buffer() throws -> UnsafeMutableBufferPointer<UInt8>? {
            var capacity: UInt32 = 0
            let ptr = try GetBuffer(&capacity)
            return UnsafeMutableBufferPointer(start: ptr, count: Int(capacity))
        }

        fileprivate func GetBuffer(_ capacity: inout UInt32) throws -> UnsafeMutablePointer<UInt8>? {
            var buffer: UnsafeMutablePointer<UInt8>?
            try perform(as: CWinRT.IMemoryBufferByteAccess.self) { pThis in
                try CHECKED(pThis.pointee.lpVtbl.pointee.GetBuffer(pThis, &buffer, &capacity))
            }
            return buffer
        }
    }

    public typealias IMemoryBufferByteAccessWrapper = InterfaceWrapperBase<IMemoryBufferByteAccessBridge>
}

public enum IMemoryBufferByteAccessBridge: AbiInterfaceBridge {
    public static func makeAbi() -> CABI {
        return CABI(lpVtbl: &IMemoryBufferByteAccessVTable)
    }

    public static func from(abi: ComPtr<CABI>?) -> SwiftProjection? {
        // This code path is not actually reachable since IMemoryBufferByteAccess is not a WinRT interface.
        // It is a COM interface which is implemented by any object which implements the IMemoryBufferReference interface.
        // And the IMemoryBufferReferenceImpl object will correctly have the implementation of this interface, so this isn't needed
        assertionFailure("IMemoryBufferByteAccessBridge.from not implemented")
        return nil
    }

    public typealias CABI = CWinRT.IMemoryBufferByteAccess
    public typealias SwiftABI = __ABI_.IMemoryBufferByteAccess
    public typealias SwiftProjection = AnyIMemoryBufferByteAccess
}

fileprivate var IMemoryBufferByteAccessVTable: CWinRT.IMemoryBufferByteAccessVtbl = .init(
    QueryInterface: {  __ABI_.IMemoryBufferByteAccessWrapper.queryInterface($0, $1, $2) },
    AddRef: {  __ABI_.IMemoryBufferByteAccessWrapper.addRef($0) },
    Release: {  __ABI_.IMemoryBufferByteAccessWrapper.release($0) },
    GetBuffer: { __ABI_.IMemoryBufferByteAccessWrapper.getBuffer($0, $1, $2) }
)

extension __ABI_.IMemoryBufferByteAccessWrapper {
    fileprivate static func getBuffer(_ this: UnsafeMutablePointer<CWinRT.IMemoryBufferByteAccess>?, _ buffer: UnsafeMutablePointer<UnsafeMutablePointer<UInt8>?>?, _ count: UnsafeMutablePointer<UInt32>?) -> HRESULT {
        guard let swiftObj = __ABI_.IMemoryBufferByteAccessWrapper.tryUnwrapFrom(raw: this) else { return E_INVALIDARG }
        guard let buffer, let count else { return E_INVALIDARG }
        count.pointee = 0
        buffer.pointee = nil

        guard let swiftBuffer = try? swiftObj.buffer else { return E_FAIL }
        count.pointee = UInt32(swiftBuffer.count)
        buffer.pointee = swiftBuffer.baseAddress
        return S_OK
    }
}