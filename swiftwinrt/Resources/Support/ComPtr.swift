// Copyright © 2023 The Browser Company
// SPDX-License-Identifier: BSD-3
import C_BINDINGS_MODULE

public class ComPtr<CInterface> {
    fileprivate var pUnk: UnsafeMutablePointer<CInterface>?

    public init(_ ptr: UnsafeMutablePointer<CInterface>?) {
        self.pUnk = ptr
        asIUnknown {
            _ = $0.pointee.lpVtbl.pointee.AddRef($0)
        }
    }

    fileprivate init() {}

    public func detach() -> UnsafeMutableRawPointer? {
        let result = pUnk
        pUnk = nil
        return UnsafeMutableRawPointer(result)
    }

    func get() -> UnsafeMutablePointer<CInterface> {
      guard let pUnk else { preconditionFailure("get() called on nil pointer") }
      return pUnk
    }

    deinit {
       release()
    }

    private func release() {
        guard let pUnk else { return }
        asIUnknown {
            _ = $0.pointee.lpVtbl.pointee.Release($0)
        }
    }

    func asIUnknown<ResultType>(_ body: (UnsafeMutablePointer<C_IUnknown>) throws -> ResultType) rethrows -> ResultType {
        guard let pUnk else { preconditionFailure("asIUnknown called on nil pointer") }
        return try pUnk.withMemoryRebound(to: C_IUnknown.self, capacity: 1) { try body($0) }
    }

    func reset<ResultType>(_ body: (inout UnsafeMutablePointer<CInterface>?) throws -> ResultType) rethrows -> ResultType {
        release()
        return try body(&pUnk)
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
        return .init(ptr)
    }
}

public struct ComPtrs {
    public static func initialize<I>(to: I.Type, _ body: (inout UnsafeMutableRawPointer?) throws -> ()) rethrows -> (ComPtr<I>) {
        let ptr = ComPtr<I>()
        try ptr.reset { (unsafePtr: inout UnsafeMutablePointer<I>?) in
            var ptrRaw: UnsafeMutableRawPointer?
            try body(&ptrRaw)
            unsafePtr = ptrRaw?.assumingMemoryBound(to: I.self)
        }
        return (ptr)
    }

    public static func initialize<I>(_ body: (inout UnsafeMutablePointer<I>?) throws -> ()) rethrows -> (ComPtr<I>) {
        let ptr = ComPtr<I>()
        try ptr.reset(body)
        return (ptr)
    }

    public static func initialize<I, I2>(_ body: (inout UnsafeMutablePointer<I>?, inout UnsafeMutablePointer<I2>?) throws -> ()) rethrows -> (ComPtr<I>, ComPtr<I2>) {
        let ptr1 = ComPtr<I>()
        let ptr2 = ComPtr<I2>()
        try ptr1.reset { ptr1Raw in
          try ptr2.reset { ptr2Raw in
            try body(&ptr1Raw, &ptr2Raw)
          }
        }
        return (ptr1, ptr2)
    }

    public static func initialize<I, I2, I3>(_ body: (inout UnsafeMutablePointer<I>?, inout UnsafeMutablePointer<I2>?, inout UnsafeMutablePointer<I3>?) throws -> ()) rethrows -> (ComPtr<I>, ComPtr<I2>, ComPtr<I3>) {
        let ptr1 = ComPtr<I>()
        let ptr2 = ComPtr<I2>()
        let ptr3 = ComPtr<I3>()
        try ptr1.reset { ptr1Raw in
          try ptr2.reset { ptr2Raw in
            try ptr3.reset { ptr3raw in
                try body(&ptr1Raw, &ptr2Raw, &ptr3raw)
            }
          }
        }
        return (ptr1, ptr2, ptr3)
    }
}
