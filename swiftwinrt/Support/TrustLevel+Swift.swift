// Copyright © 2021 Saleem Abdulrasool <compnerd@compnerd.org>
// SPDX-License-Identifier: BSD-3

import Ctest_component

extension TrustLevel {
  public static var BaseTrust: TrustLevel {
    TrustLevel(rawValue: 0)
  }

  public static var PartialTrust: TrustLevel {
    TrustLevel(rawValue: 1)
  }

  public static var FullTrust: TrustLevel {
    TrustLevel(rawValue: 2)
  }
}
