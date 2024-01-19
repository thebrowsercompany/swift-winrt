import C_BINDINGS_MODULE

fileprivate let IID_IMemoryBufferByteAccess = IID(Data1: 0x5B0D3235 Data2: 0x4DBA, Data3: 0x4D44, Data4: ( 0x86, 0x5E, 0x8F, 0x1D, 0x0E, 0x4F, 0xD0, 0x4D )) // 5b0d3235-4dba-4d44-865e-8f1d0e4fd04d

class IMemoryBufferByteAccess: IUnknown {
    override class var IID: IID { IID_IMemoryBufferByteAccess}

    func Buffer(_ bytes: UnsafeMutablePointer<UInt8>?) throws {
        var buffer: UnsafeMutablePointer<UInt8>?
        try perform(as: C_BINDINGS_MODULE.IMemoryBufferByteAccess.self) { pThis in
            try CHECKED(pThis.pointee.lpVtbl.pointee.Buffer(pThis, &buffer))
        }
    }
}