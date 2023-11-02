// Copyright © 2023 The Browser Company
// SPDX-License-Identifier: BSD-3
import C_BINDINGS_MODULE

open class ComObject<CInterface>: IUnknown2 {
  public let ptr: ComPtr<CInterface>
  public required init(_ pointer: ComPtr<CInterface>) {
    self.ptr = pointer
  }

  @_spi(ComObject)
  public var thisPtr: UnsafeMutablePointer<CInterface> { self.ptr.get() }

  open class var IID: test_component.IID { fatalError("class IID not implemented") }

  public func QueryInterface(_ iid: REFIID, _ ppvObject: UnsafeMutablePointer<UnsafeMutableRawPointer?>?) throws {
    try self.ptr.asIUnknown {
      try CHECKED($0.pointee.lpVtbl.pointee.QueryInterface($0, iid, ppvObject))
    }
  }

  @discardableResult
  public func AddRef() -> ULONG {
    self.ptr.asIUnknown {
      $0.pointee.lpVtbl.pointee.AddRef($0)
    }
  }

  @discardableResult
  public func Release() -> ULONG {
    self.ptr.asIUnknown {
      $0.pointee.lpVtbl.pointee.Release($0)
    }
  }
}

public extension ComPtr {
    func queryInterface<CInterface2>() throws -> ComObject<CInterface2> {
        let ptr = try self.asIUnknown { pUnk in
            var iid = ComObject<CInterface2>.IID
            return try ComPtrs.initialize(to: CInterface2.self) { result in
                try CHECKED(pUnk.pointee.lpVtbl.pointee.QueryInterface(pUnk, &iid, &result))
            }
        }
        return .init(ptr)
    }
}
