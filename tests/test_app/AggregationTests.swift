import WinSDK
import XCTest
@_spi(WinRTInternal) import test_component
import Ctest_component
import Foundation

class AggregationTests : XCTestCase {

  public func testAppOverridesFromBase() throws {
    let derived = Derived()
    try derived.doTheThing()

    let appDerived = AppDerived()
    print(type(of: appDerived))

    let b: Base = appDerived as Base
    print(type(of: b))

    try appDerived.doTheThing()
    XCTAssertEqual(appDerived.count, 1, "count not expected")
  }

  public func testAppOverridesFromDerived() throws {
    let appDerived2 = AppDerived2()
    try appDerived2.doTheThing()
    XCTAssertEqual(appDerived2.count, 1, "count not expected")
  }

  public func testOverrideMultipleInterface() throws {
    let appDerived3 = AppDerived3()
    try appDerived3.doTheThing()
    XCTAssertEqual(appDerived3.count, 1, "count not expected")
    XCTAssertEqual(appDerived3.beforeCount, 1, "before count not expected")
  }

  public func testComposedTypesAsInput() throws {
    let appDerived = AppDerived()
    try StaticClass.takeBase(appDerived)
    XCTAssertEqual(appDerived.count, 1, "derive from base count not expected")

    let appDerived2 = AppDerived2()

    try StaticClass.takeBase(appDerived2)
    XCTAssertEqual(appDerived2.count, 1, "derived from derived count not expected")

    let appDerived3 = AppDerived3()
    try StaticClass.takeBase(appDerived3)
    XCTAssertEqual(appDerived3.count, 1, "derived from multiple interfaces count not expected")
    XCTAssertEqual(appDerived3.beforeCount, 1, "before count not expected")
  }

  public func testUnwrappingWinRTImplementedComposedType() throws {
    let classy = Class()
    let base_returned = classy.baseProperty!
    XCTAssert(type(of: base_returned) == Derived.self)
  }

  public func testUnwrappingAppImplementedComposedTypeBase() throws {
    let classy = Class()
    let appDerived = AppDerived()
    classy.baseProperty = appDerived

    let base_returned = try XCTUnwrap(classy.baseProperty)
    XCTAssert(type(of: base_returned) == AppDerived.self, "type doesn't match!")
    XCTAssertIdentical(base_returned, appDerived)
  }

  public func testUnwrappingAppImplementedComposedTypeDerive() throws {
    let classy = Class()
    let appDerived2 = AppDerived2()

    classy.baseProperty = appDerived2

    let base_returned = try XCTUnwrap(classy.baseProperty)
    XCTAssert(type(of: base_returned) == AppDerived2.self)
    XCTAssertIdentical(base_returned, appDerived2)
  }

  public func testUnwrappingAppImplementedComposedTypeDerive3() throws {
    let classy = Class()
    let appDerived3 = AppDerived3()

    classy.baseProperty = appDerived3

    let base_returned = try XCTUnwrap(classy.baseProperty)
    XCTAssert(type(of: base_returned) == AppDerived3.self)
    XCTAssertIdentical(base_returned, appDerived3)
  }

  // Verifies that a composable type with no override interfaces still works
  // as expected
  public func testUnwrappingAppImplementedComposedFromBaseNoOverrides() throws {
    let classy = Class()

    let derivedNoOverrides = AppDerivedNoOverrides()
    classy.baseNoOverridesProperty = derivedNoOverrides

    let baseNoOverrides_returned = try XCTUnwrap(classy.baseNoOverridesProperty)
    XCTAssert(type(of: baseNoOverrides_returned) == AppDerivedNoOverrides.self)
    XCTAssertIdentical(baseNoOverrides_returned, derivedNoOverrides)
  }

  public func testUnwrappingAppImplementedComposedFromDerivedNoOverrides() throws {
    let classy = Class()

    let derivedNoOverrides2 = AppDerivedNoOverrides2()
    classy.baseNoOverridesProperty = derivedNoOverrides2
    let baseNoOverrides_returned = try XCTUnwrap(classy.baseNoOverridesProperty)
    XCTAssert(type(of: baseNoOverrides_returned) == AppDerivedNoOverrides2.self)
    XCTAssertIdentical(baseNoOverrides_returned, derivedNoOverrides2)
  }

  public func testCustomConstructorOnUnsealedType() throws {
    let derived = UnsealedDerived(32)
    XCTAssertEqual(derived.prop, 32)
  }

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

  public func testAggregatedObjectUnwrappedAsAny() throws {
    let derived = AppDerived()
    try Class.takeBaseAndGiveToCallbackAsObject(derived) { sender in
      let base = sender as? AppDerived
      XCTAssertNotNil(derived)
      XCTAssertIdentical(base, derived)
    }
  }
}

var aggregationTests: [XCTestCaseEntry] = [
  testCase([
    ("testAppOverridesFromBase", AggregationTests.testAppOverridesFromBase),
    ("testAppOverridesFromDerived", AggregationTests.testAppOverridesFromDerived),
    ("testOverrideMultipleInterface", AggregationTests.testOverrideMultipleInterface),
    ("testComposedTypesAsInput", AggregationTests.testComposedTypesAsInput),
    ("testUnwrappingWinRTImplementedComposedType", AggregationTests.testUnwrappingWinRTImplementedComposedType),
    ("testUnwrappingAppImplementedComposedTypeBase", AggregationTests.testUnwrappingAppImplementedComposedTypeBase),
    ("testUnwrappingAppImplementedComposedTypeDerive", AggregationTests.testUnwrappingAppImplementedComposedTypeDerive),
    ("testUnwrappingAppImplementedComposedTypeDerive3", AggregationTests.testUnwrappingAppImplementedComposedTypeDerive3),
    ("testUnwrappingAppImplementedComposedFromBaseNoOverrides", AggregationTests.testUnwrappingAppImplementedComposedFromBaseNoOverrides),
    ("testUnwrappingAppImplementedComposedFromDerivedNoOverrides", AggregationTests.testUnwrappingAppImplementedComposedFromDerivedNoOverrides),
    ("testCustomConstructorOnUnsealedType", AggregationTests.testCustomConstructorOnUnsealedType),
    ("testGetRuntimeClassNameReturnsBase", AggregationTests.testGetRuntimeClassNameReturnsBase),
    ("testAggregatedObjectUnwrappedAsAny", AggregationTests.testAggregatedObjectUnwrappedAsAny),
  ])
]
