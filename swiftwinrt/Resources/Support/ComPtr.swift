// Copyright © 2023 The Browser Company
// SPDX-License-Identifier: BSD-3
import C_BINDINGS_MODULE

// ComPtr is a smart pointer for COM interfaces.  It holds onto the underlying pointer
// and the semantics of it are meant to mirror that of the ComPtr class in WRL. The
// design of ComPtr and ComPtrs.intialize is that there should be no use of UnsafeMutablePointer
// any where else in the code base.  The only place where UnsafeMutablePointer should be used is
// where it's required at the ABI boundary.
public class ComPtr<CInterface> {
    fileprivate var pUnk: UnsafeMutablePointer<CInterface>?

    public init(_ ptr: UnsafeMutablePointer<CInterface>) {
        self.pUnk = ptr
        asIUnknown {
            _ = $0.pointee.lpVtbl.pointee.AddRef($0)
        }
    }

    public convenience init?(_ ptr: UnsafeMutablePointer<CInterface>?) {
        guard let ptr else { return nil }
        self.init(ptr)
    }

    // Release ownership of the underlying pointer and return it. This is
    // useful when assigning to an out parameter and avoids an extra Add/Ref
    // release call.
    public func detach() -> UnsafeMutableRawPointer? {
        let result = pUnk
        pUnk = nil
        return UnsafeMutableRawPointer(result)
    }

    public func get() -> UnsafeMutablePointer<CInterface> {
      guard let pUnk else { preconditionFailure("get() called on nil pointer") }
      return pUnk
    }

    deinit {
       release()
    }

    private func release() {
        guard pUnk != nil else { return }
        asIUnknown {
            _ = $0.pointee.lpVtbl.pointee.Release($0)
        }
    }

    func asIUnknown<ResultType>(_ body: (UnsafeMutablePointer<C_IUnknown>) throws -> ResultType) rethrows -> ResultType {
        guard let pUnk else { preconditionFailure("asIUnknown called on nil pointer") }
        return try pUnk.withMemoryRebound(to: C_IUnknown.self, capacity: 1) { try body($0) }
    }
}

public extension ComPtr {
    func queryInterface<Interface: IUnknown>() throws -> Interface {
        let ptr = try self.asIUnknown { pUnk in
            var iid = Interface.IID
            return try ComPtrs.initialize(to: C_IUnknown.self) { result in
                try CHECKED(pUnk.pointee.lpVtbl.pointee.QueryInterface(pUnk, &iid, &result))
            }
        }
        return .init(ptr!)
    }
}

// ComPtrs properly initializes pointers who have ownership of the underlying raw pointers. This is used at the ABI boundary layer, for example:
// let (return1, return2) = try ComPtrs.initialize { return1Abi, return2Abi in
//    try CHECKED(pThis.pointee.lpVtbl.pointee.Method(pThis, &return1Abi, &return2Abi))
// }
public struct ComPtrs {
    // Note: The single initialization methods still return a tuple for ease of code generation
    public static func initialize<I>(to: I.Type, _ body: (inout UnsafeMutableRawPointer?) throws -> ()) rethrows -> (ComPtr<I>?) {
        var ptrRaw: UnsafeMutableRawPointer?
        try body(&ptrRaw)
        return (ComPtr(ptrRaw?.assumingMemoryBound(to: I.self)))
    }

    public static func initialize<I>(_ body: (inout UnsafeMutablePointer<I>?) throws -> ()) rethrows -> (ComPtr<I>?) {
        var ptr: UnsafeMutablePointer<I>?
        try body(&ptr)
        return (ComPtr(ptr))
    }

    public static func initialize<I, I2>(_ body: (inout UnsafeMutablePointer<I>?, inout UnsafeMutablePointer<I2>?) throws -> ()) rethrows -> (ComPtr<I>?, ComPtr<I2>?) {
        var ptr1: UnsafeMutablePointer<I>?
        var ptr2: UnsafeMutablePointer<I2>?
        try body(&ptr1, &ptr2)
        return (ComPtr(ptr1), ComPtr(ptr2))
    }

    public static func initialize<I, I2, I3>(_ body: (inout UnsafeMutablePointer<I>?, inout UnsafeMutablePointer<I2>?, inout UnsafeMutablePointer<I3>?) throws -> ()) rethrows -> (ComPtr<I>?, ComPtr<I2>?, ComPtr<I3>?) {
        var ptr1: UnsafeMutablePointer<I>?
        var ptr2: UnsafeMutablePointer<I2>?
        var ptr3: UnsafeMutablePointer<I3>?
        try body(&ptr1, &ptr2, &ptr3)
        return (ComPtr(ptr1), ComPtr(ptr2), ComPtr(ptr3))
    }
}
