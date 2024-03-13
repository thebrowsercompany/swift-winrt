
import Foundation
import XCTest
import test_component

extension URL {
    func asFSR() -> String {
        assert(isFileURL, "URL must be a file URL")
        return withUnsafeFileSystemRepresentation { String(cString: $0!) }
    }
}

class CustomBuffer: IBuffer {
    public var length: UInt32 = 0 {
        willSet {
            if newValue > capacity {
                fatalError("Buffer length cannot be greater than capacity")
            }
        }
    }

    public var capacity: UInt32 {
        return UInt32(_buffer.count)
    }
    public let _buffer: UnsafeMutableBufferPointer<UInt8>

    public init(_ capacity: Int) {
        self._buffer = UnsafeMutableBufferPointer.allocate(capacity: capacity)
    }

    public var buffer: UnsafeMutablePointer<UInt8>? {
        return _buffer.baseAddress
    }

    deinit {
        _buffer.deallocate()
    }
}

class BufferTests : XCTestCase {
    public func testWinRTBuffer() throws {
        let buffer = Buffer(10)
        XCTAssertEqual(buffer.data.count, 0)
        XCTAssertEqual(buffer.capacity, 10)
        buffer.length = 10
        XCTAssertEqual(buffer.capacity, 10)
        XCTAssertEqual(buffer.data.count, 10)
        for (i, byte) in buffer.data.enumerated() {
            XCTAssertEqual(byte, try BufferTester.getDataFrom(buffer, UInt32(i)))
        }
    }

    public func testSwiftBuffer() async throws {
        // Picking an arbitrary file and comparing a custom swift implementation of IBuffer
        // to the built-in WinRT one
        let path = Bundle.main.bundleURL.appendingPathComponent("test_app.exe.manifest").asFSR()
        let file = try await StorageFile.getFileFromPathAsync(path).get()
        let stream = try await file!.openAsync(.read).get()

        let swiftBuffer = CustomBuffer(500)
        let winrtBuffer = Buffer(500)
        _ = try await stream!.readAsync(swiftBuffer, 500, .none).get()
        try stream!.seek(0) // seek back to beginning so we read the same data again
        _ = try await stream!.readAsync(winrtBuffer, 500, .none).get()

        XCTAssertEqual(swiftBuffer.length, winrtBuffer.length)
        XCTAssertEqual(swiftBuffer.data, winrtBuffer.data)

        for (i, byte) in swiftBuffer.data.enumerated() {
            XCTAssertEqual(byte, try BufferTester.getDataFrom(swiftBuffer, UInt32(i)))
        }
    }
}

var bufferTests: [XCTestCaseEntry] = [
  testCase([
    ("testWinRTBuffer", BufferTests.testWinRTBuffer),
    ("testSwiftBuffer", asyncTest(BufferTests.testSwiftBuffer)),
  ])
]
