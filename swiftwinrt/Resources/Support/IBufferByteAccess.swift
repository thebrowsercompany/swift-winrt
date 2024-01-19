import C_BINDINGS_MODULE
import Foundation

fileprivate let IID_IBufferByteAccess = IID(Data1: 0x905A0FEF, Data2: 0xBC53, Data3: 0x11DF, Data4: ( 0x8C, 0x49, 0x00, 0x1E, 0x4F, 0xC6, 0x86, 0xDA )) // 905a0fef-bc53-11df-8c49-001e4fc686da

fileprivateclass IBufferByteAccessABI: IUnknown {
    override class var IID: IID { IID_IBufferByteAccess}

    func Buffer(_ bytes: UnsafeMutablePointer<UInt8>?) throws {
        var buffer: UnsafeMutablePointer<UInt8>?
        try perform(as: C_BINDINGS_MODULE.IBufferByteAccess.self) { pThis in
            try CHECKED(pThis.pointee.lpVtbl.pointee.Buffer(pThis, &buffer))
        }
    }
}

public protocol IBufferByteAccess: IWinRTObject {
    var data: Data { get }
}
