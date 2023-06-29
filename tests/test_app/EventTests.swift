import WinSDK
import XCTest
import test_component
import Foundation

extension Array where Element == Disposable? {
  mutating func dispose() {
    for item in self {
      if let item {
        item.dispose()
      }
    }
    self.removeAll(keepingCapacity: true)
  }
}

class EventTests : XCTestCase {
  public func testEventsOnInstance() throws {
    let simple = Simple()
    var count = 0
    
    var disposable = [Disposable?]()

    disposable.append(simple.signalEvent.addHandler {
      count+=1
    })

    try simple.fireEvent()
    XCTAssertEqual(count, 1)

    try simple.fireEvent()
    XCTAssertEqual(count, 2)

    // dispose of the handlers and make sure we
    // aren't getting more events
    disposable.dispose()

    try simple.fireEvent()
    XCTAssertEqual(count, 2)

    // hookup the handlers again and make sure it works, just to be safe
    simple.signalEvent.addHandler {
      count+=1
    }

    try simple.fireEvent()
    XCTAssertEqual(count, 3)
  }

  public func testStaticEvent() throws {
    var static_count = 0
    
    var disposable = [Disposable?]()

    disposable.append(Simple.staticEvent.addHandler { (_,_) in
      static_count+=1
    })

    Simple.fireStaticEvent()
    XCTAssertEqual(static_count, 1)
    Simple.fireStaticEvent()
    XCTAssertEqual(static_count, 2)

    // dispose of the handlers and make sure we
    // aren't getting more events
    disposable.dispose()

    Simple.fireStaticEvent()
    XCTAssertEqual(static_count, 2)

    disposable.append(Simple.staticEvent.addHandler { (_,_) in
      static_count+=1
    })

    Simple.fireStaticEvent()
    XCTAssertEqual(static_count, 3)
  }

  public func testSwiftImplementedEventWithWinRTListener() throws {
    let delegate = MyImplementableDelegate()
    let tester = EventTester(delegate)
    try tester.subscribe()
    var expectedResult = "fired once"
    delegate.fireEvent(expectedResult)
    XCTAssertEqual(tester.count, 1)
    XCTAssertEqual(try tester.getResult(), expectedResult)

    expectedResult = "fired twice"
    delegate.fireEvent(expectedResult)
    XCTAssertEqual(tester.count, 2)
    XCTAssertEqual(try tester.getResult(), expectedResult)
  
    try tester.unsubscribe()
    delegate.fireEvent("unexpected shouldn't get this")
    XCTAssertEqual(tester.count, 2)
    XCTAssertEqual(try tester.getResult(), expectedResult)

    try tester.subscribe()
    expectedResult = "fired trice"
    delegate.fireEvent(expectedResult)
    XCTAssertEqual(tester.count, 3)
    XCTAssertEqual(try tester.getResult(), expectedResult)
  }

public func testSwiftImplementedEventWithSwiftListener() throws {
    let delegate = MyImplementableDelegate()
    var count = 0
    var expectedResult = "fired once"
    let disposer = delegate.implementableEvent.addHandler{
      count+=1
      XCTAssertEqual($0, expectedResult)
    }

    delegate.fireEvent(expectedResult)
    XCTAssertEqual(count, 1)
    expectedResult = "fired twice"
    delegate.fireEvent(expectedResult)
    XCTAssertEqual(count, 2)
  
    disposer?.dispose()
    delegate.fireEvent("unexpected shouldn't get this")
    XCTAssertEqual(count, 2)

    delegate.implementableEvent.addHandler{
      count+=1
      XCTAssertEqual($0, expectedResult)
    }
    expectedResult = "fired trice"
    delegate.fireEvent(expectedResult)
    XCTAssertEqual(count, 3)
  }
}

var eventTests: [XCTestCaseEntry] = [
  testCase([
    ("EventsOnInstance", EventTests.testEventsOnInstance),
    ("StaticEvent", EventTests.testStaticEvent),
    ("SwiftImplementedEventWithWinRTListener", EventTests.testSwiftImplementedEventWithWinRTListener),
    ("SwiftImplementedEventWithSwiftListener", EventTests.testSwiftImplementedEventWithSwiftListener),
  ])
]