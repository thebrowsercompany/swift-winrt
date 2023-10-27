import WinSDK
import XCTest
@_spi(WinRTInternal) import test_component
import Ctest_component
import Foundation

class AggregationTests : XCTestCase {

  public func testAggregation() throws {
    let derived = Derived()
    try derived.doTheThing()

    let appDerived = AppDerived()
    print(type(of: appDerived))

    let b: Base = appDerived as Base
    print(type(of: b))

    try appDerived.doTheThing()
    XCTAssertEqual(appDerived.count, 1, "1. count not expected")

    print("foo");
    let appDerived2 = AppDerived2()
    print("foo");
    try appDerived2.doTheThing()
    print("foo");
    XCTAssertEqual(appDerived2.count, 1, "2. count not expected")

    print("foo");
    let appDerived3 = AppDerived3()
    try appDerived3.doTheThing()
    XCTAssertEqual(appDerived3.count, 1, "3. count not expected")
    XCTAssertEqual(appDerived3.beforeCount, 1, "4. count not expected")

    StaticClass.takeBase(appDerived)
    XCTAssertEqual(appDerived.count, 2, "5. count not expected")

    StaticClass.takeBase(appDerived2)
    XCTAssertEqual(appDerived2.count, 2, "6. count not expected")

    StaticClass.takeBase(appDerived3)
    XCTAssertEqual(appDerived3.count, 2, "7. count not expected")
    XCTAssertEqual(appDerived3.beforeCount, 2, "8. count not expected")


    print("testing unwrapping proper types from return values")
    let classy = Class()
    var base_returned = classy.baseProperty!
    XCTAssert(type(of: base_returned) == Derived.self)

    print("testing app derived")
    classy.baseProperty = appDerived

    base_returned = classy.baseProperty
    XCTAssert(type(of: base_returned) == AppDerived.self)
    XCTAssertIdentical(base_returned, appDerived)

    print("testing app derived2")

    classy.baseProperty = appDerived2

    base_returned = classy.baseProperty
    XCTAssert(type(of: base_returned) == AppDerived2.self)
    XCTAssertIdentical(base_returned, appDerived2)

    print("testing app derived3")

    classy.baseProperty = appDerived3

    base_returned = classy.baseProperty
    XCTAssert(type(of: base_returned) == AppDerived3.self)
    XCTAssertIdentical(base_returned, appDerived3)

    print("testing app derived no overrides")

    let derivedNoOverrides = AppDerivedNoOverrides()
    classy.baseNoOverridesProperty = derivedNoOverrides

    var baseNoOverrides_returned = classy.baseNoOverridesProperty!
    XCTAssert(type(of: baseNoOverrides_returned) == AppDerivedNoOverrides.self)
    XCTAssertIdentical(baseNoOverrides_returned, derivedNoOverrides)

    let derivedNoOverrides2 = AppDerivedNoOverrides2()
    classy.baseNoOverridesProperty = derivedNoOverrides2
    baseNoOverrides_returned = classy.baseNoOverridesProperty
    XCTAssert(type(of: baseNoOverrides_returned) == AppDerivedNoOverrides2.self)
    XCTAssertIdentical(baseNoOverrides_returned, derivedNoOverrides2)
  }

  public func testCustomConstructorOnUnsealedType() throws {
    let derived = UnsealedDerived(32)
    XCTAssertEqual(derived.prop, 32)
  }

  // Test requires a function which takes a Base type. This ensures
  // we aren't doing any we
  public func runtimeClassName(_ base: Base) -> String {
    return String(hString: base.GetRuntimeClassName())
  }

  public func testGetRuntimeClassNameReturnsBase() throws {
    // WinUI calls GetRuntimeClassName when types are being constructed,
    // so we need to return something that the runtime understands. Without an
    // IXamlMetadataProvider implementation, returning custom type names will
    // break the app.
    let appDerived = AppDerived()

    // In order to validate the test, we have to call GetRuntimeClassName on the base
    // type. This will make the type check we had think we're being aggregated, where
    // calling it directly on the AppDerived type won't
    XCTAssertEqual(runtimeClassName(appDerived), "test_component.Base")
  }
}

var aggregationTests: [XCTestCaseEntry] = [
  testCase([
    ("testAggregation", AggregationTests.testAggregation),
    ("testCustomConstructorOnUnsealedType", AggregationTests.testCustomConstructorOnUnsealedType),
    ("testGetRuntimeClassNameReturnsBase", AggregationTests.testGetRuntimeClassNameReturnsBase)
  ])
]
