// Copyright © 2021 Saleem Abdulrasool <compnerd@compnerd.org>
// SPDX-License-Identifier: BSD-3

import WinSDK

public typealias REFIID = UnsafePointer<IID>

// winnt.h

@_transparent
internal func MAKELANGID(_ p: WORD, _ s: WORD) -> DWORD {
  return DWORD((s << 10) | p)
}

extension boolean {
  public init(from val: Bool) {
    let value : boolean = val ? 1 : 0
    self.init(value)
  }
}

extension HWND {
  public init?(from val: UInt64) {
    self.init(HWND(bitPattern: UInt(val)))
  }
}

extension WCHAR {
  public init(from val: Character) {
    let value = try! String(val).withWideChars {
      $0[0]
    }
    self.init(value);
  }
}

// stdlib.h
public var MB_MAX : Int { 
  Int(___mb_cur_max_func())
}
