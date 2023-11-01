import WinSDK
import XCTest
import test_component
import Foundation

class AsyncTests : XCTestCase {
  public func testAwaitAlreadyCompleted() throws {
    let asyncOperation = AsyncMethods.getCompletedAsync(42)!
    XCTAssertEqual(asyncOperation.status, .completed)
    let result = try asyncBlock(timeout: 0.1) { try await asyncOperation.get() }
    XCTAssertEqual(result, 42)
  }

  public func testAwaitAlreadyFailed() throws {
    let asyncOperation = AsyncMethods.getCompletedWithErrorAsync(E_LAYOUTCYCLE)!
    XCTAssertEqual(asyncOperation.status, .error)
    do {
      _ = try asyncBlock(timeout: 0.1) { try await asyncOperation.get() }
      XCTFail("Expected an error to be thrown")
    } catch let error as test_component.Error {
      XCTAssertEqual(error.hr, E_LAYOUTCYCLE)
    }
  }

  public func testAwaitCompletionWithSuspension() throws {
    let asyncOperation = AsyncMethods.getPendingAsync()!
    runAfter(delay: 0.05) { try? asyncOperation.complete(42) }
    let result = try asyncBlock(timeout: 0.1) { try await asyncOperation.get() }
    XCTAssertEqual(asyncOperation.status, .completed)
    XCTAssertEqual(result, 42)
  }

  public func testAwaitFailureWithSuspension() throws {
    let asyncOperation = AsyncMethods.getPendingAsync()!
    runAfter(delay: 0.05) { try? asyncOperation.completeWithError(E_LAYOUTCYCLE) }
    do {
      _ = try asyncBlock(timeout: 0.1) { try await asyncOperation.get() }
      XCTFail("Expected an error to be thrown")
    } catch let error as test_component.Error {
      XCTAssertEqual(error.hr, E_LAYOUTCYCLE)
    }
  }

  /// Runs a block asynchronously after a delay has elapsed
  private func runAfter(delay: TimeInterval, body: @escaping () -> Void) {
    Task {
      try await Task.sleep(nanoseconds: UInt64(delay * 1_000_000_000))
      body()
    }
  }

  private class BoxedResult<T> {
    var result: Result<T, any Swift.Error>? = nil
  } 

  /// Synchronously runs an async block with a timeout
  private func asyncBlock<Result>(timeout: TimeInterval, body: @escaping () async throws -> Result) throws -> Result {
    guard let event = WinSDK.CreateEventA(nil, false, false, nil) else {
      XCTFail("Failed to create event")
      throw test_component.Error(hr: E_FAIL)
    }
    defer { WinSDK.CloseHandle(event) }

    let asyncResult = BoxedResult<Result>()
    Task {
      do {
        asyncResult.result = .success(try await body())
      } catch {
        asyncResult.result = .failure(error)
      }
      WinSDK.SetEvent(event)
    }

    switch WinSDK.WaitForSingleObject(event, /* dwMilliseconds: */ DWORD(timeout * 1000)) {
      case WAIT_OBJECT_0: break
      case DWORD(WAIT_TIMEOUT):
        XCTFail("Timed out waiting for async task to complete")
        throw test_component.Error(hr: E_FAIL)
      default:
        XCTFail("Failed to wait for async task to complete")
        throw test_component.Error(hr: E_FAIL)
    }

    return try asyncResult.result!.get()
  }
}

var asyncTests: [XCTestCaseEntry] = [
  testCase([
    ("testAwaitAlreadyCompleted", AsyncTests.testAwaitAlreadyCompleted),
    ("testAwaitAlreadyFailed", AsyncTests.testAwaitAlreadyFailed),
    ("testAwaitCompletionWithSuspension", AsyncTests.testAwaitCompletionWithSuspension),
    ("testAwaitFailureWithSuspension", AsyncTests.testAwaitFailureWithSuspension),
  ])
]
