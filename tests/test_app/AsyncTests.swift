import WinSDK
import XCTest
import test_component
import Foundation

class AsyncTests : XCTestCase {
  public func testAwaitAlreadyCompleted() throws {
    try runSync {
      let asyncOperation = AsyncMethods.getCompletedAsync(42)!
      XCTAssertEqual(asyncOperation.status, .completed)
      let result = try await asyncOperation.get()
      XCTAssertEqual(result, 42)
    }
  }

  public func testAwaitAlreadyFailed() throws {
    try runSync {
      let asyncOperation = AsyncMethods.getCompletedWithErrorAsync(-42)!
      XCTAssertEqual(asyncOperation.status, .error)
      do {
        let _ = try await asyncOperation.get()
        XCTFail("Expected an error to be thrown")
      }
      catch {}
    }
  }

  func runSync(body: @escaping () async throws -> Void) throws {
    guard let event = WinSDK.CreateEventA(nil, false, false, nil) else {
      XCTFail("Failed to create event")
      return
    }
    defer { WinSDK.CloseHandle(event) }

    class Result { var error: (any Swift.Error)? = nil }
    let result = Result()
    Task {
      do { try await body() }
      catch let caught { result.error = caught }
      WinSDK.SetEvent(event)
    }

    switch WinSDK.WaitForSingleObject(event, /* dwMilliseconds: */ 1000) {
      case WAIT_OBJECT_0: break
      case DWORD(WAIT_TIMEOUT):
        XCTFail("Timed out waiting for async task to complete")
        return
      default:
        XCTFail("Failed to wait for async task to complete")
        return
    }

    if let error = result.error { throw error }
  }
}

var asyncTests: [XCTestCaseEntry] = [
  testCase([
    ("testAwaitAlreadyCompleted", AsyncTests.testAwaitAlreadyCompleted),
    ("testAwaitAlreadyFailed", AsyncTests.testAwaitAlreadyFailed),
  ])
]
