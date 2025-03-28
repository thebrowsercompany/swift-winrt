// WARNING: Please don't edit this file. It was generated by Swift/WinRT v0.0.1
// swiftlint:disable all
import Foundation
import Ctest_component

/// [Open Microsoft documentation](https://learn.microsoft.com/uwp/api/windows.storage.streams.inputstreamoptions)
public typealias InputStreamOptions = __x_ABI_CWindows_CStorage_CStreams_CInputStreamOptions
/// [Open Microsoft documentation](https://learn.microsoft.com/uwp/api/windows.storage.streams.unicodeencoding)
public typealias UnicodeEncoding = __x_ABI_CWindows_CStorage_CStreams_CUnicodeEncoding
/// [Open Microsoft documentation](https://learn.microsoft.com/uwp/api/windows.storage.streams.buffer)
public final class Buffer : WinRTClass, IBufferByteAccess, IBuffer {
    private typealias SwiftABI = __ABI_Windows_Storage_Streams.IBuffer
    private typealias CABI = __x_ABI_CWindows_CStorage_CStreams_CIBuffer
    private lazy var _default: SwiftABI! = getInterfaceForCaching()
    @_spi(WinRTInternal)
    override public func _getABI<T>() -> UnsafeMutablePointer<T>? {
        if T.self == CABI.self {
            return RawPointer(_default)
        }
        return super._getABI()
    }

    @_spi(WinRTInternal)
    public init(fromAbi: test_component.IInspectable) {
        super.init(fromAbi)
    }

    override public func queryInterface(_ iid: test_component.IID) -> IUnknownRef? {
        return super.queryInterface(iid)
    }
    private static let _IBufferFactory: __ABI_Windows_Storage_Streams.IBufferFactory = try! RoGetActivationFactory("Windows.Storage.Streams.Buffer")
    public init(_ capacity: UInt32) {
        super.init(try! Self._IBufferFactory.Create(capacity))
    }

    private static let _IBufferStatics: __ABI_Windows_Storage_Streams.IBufferStatics = try! RoGetActivationFactory("Windows.Storage.Streams.Buffer")
    /// [Open Microsoft documentation](https://learn.microsoft.com/uwp/api/windows.storage.streams.buffer.createcopyfrommemorybuffer)
    public static func createCopyFromMemoryBuffer(_ input: test_component.AnyIMemoryBuffer!) throws -> Buffer! {
        return try _IBufferStatics.CreateCopyFromMemoryBuffer(input)
    }

    /// [Open Microsoft documentation](https://learn.microsoft.com/uwp/api/windows.storage.streams.buffer.creatememorybufferoveribuffer)
    public static func createMemoryBufferOverIBuffer(_ input: AnyIBuffer!) throws -> test_component.MemoryBuffer! {
        return try _IBufferStatics.CreateMemoryBufferOverIBuffer(input)
    }

    private lazy var _IBufferByteAccess: __ABI_.IBufferByteAccess! = getInterfaceForCaching()
    public var buffer: UnsafeMutablePointer<UInt8>? {
        get throws {
            let bufferByteAccess: test_component.__ABI_.IBufferByteAccess = try _IBufferByteAccess.QueryInterface()
            return try bufferByteAccess.Buffer()
        }
    }
    /// [Open Microsoft documentation](https://learn.microsoft.com/uwp/api/windows.storage.streams.buffer.capacity)
    public var capacity : UInt32 {
        get { try! _default.get_Capacity() }
    }

    /// [Open Microsoft documentation](https://learn.microsoft.com/uwp/api/windows.storage.streams.buffer.length)
    public var length : UInt32 {
        get { try! _default.get_Length() }
        set { try! _default.put_Length(newValue) }
    }

    deinit {
        _IBufferByteAccess = nil
        _default = nil
    }
}

/// [Open Microsoft documentation](https://learn.microsoft.com/uwp/api/windows.storage.streams.ibuffer)
public protocol IBuffer : IBufferByteAccess {
    /// [Open Microsoft documentation](https://learn.microsoft.com/uwp/api/windows.storage.streams.ibuffer.capacity)
    var capacity: UInt32 { get }
    /// [Open Microsoft documentation](https://learn.microsoft.com/uwp/api/windows.storage.streams.ibuffer.length)
    var length: UInt32 { get set }
}

extension IBuffer {
    public func queryInterface(_ iid: test_component.IID) -> IUnknownRef? {
        switch iid {
            case __ABI_Windows_Storage_Streams.IBufferWrapper.IID:
                let wrapper = __ABI_Windows_Storage_Streams.IBufferWrapper(self)
                return wrapper!.queryInterface(iid)
            case __ABI_.IBufferByteAccessWrapper.IID:
                let wrapper = __ABI_.IBufferByteAccessWrapper(self)
                return wrapper!.queryInterface(iid)
            default: return nil
        }
    }
}
extension IBuffer {
    public var data: Data {
        guard let buffer = try? buffer else { return Data() }
        return Data(bytesNoCopy: buffer, count: Int(length), deallocator: .none)
    }
}
public typealias AnyIBuffer = any IBuffer

/// [Open Microsoft documentation](https://learn.microsoft.com/uwp/api/windows.storage.streams.icontenttypeprovider)
public protocol IContentTypeProvider : WinRTInterface {
    /// [Open Microsoft documentation](https://learn.microsoft.com/uwp/api/windows.storage.streams.icontenttypeprovider.contenttype)
    var contentType: String { get }
}

extension IContentTypeProvider {
    public func queryInterface(_ iid: test_component.IID) -> IUnknownRef? {
        switch iid {
            case __ABI_Windows_Storage_Streams.IContentTypeProviderWrapper.IID:
                let wrapper = __ABI_Windows_Storage_Streams.IContentTypeProviderWrapper(self)
                return wrapper!.queryInterface(iid)
            default: return nil
        }
    }
}
public typealias AnyIContentTypeProvider = any IContentTypeProvider

/// [Open Microsoft documentation](https://learn.microsoft.com/uwp/api/windows.storage.streams.iinputstream)
public protocol IInputStream : test_component.IClosable {
    /// [Open Microsoft documentation](https://learn.microsoft.com/uwp/api/windows.storage.streams.iinputstream.readasync)
    func readAsync(_ buffer: test_component.AnyIBuffer!, _ count: UInt32, _ options: test_component.InputStreamOptions) throws -> test_component.AnyIAsyncOperationWithProgress<test_component.AnyIBuffer?, UInt32>!
}

extension IInputStream {
    public func queryInterface(_ iid: test_component.IID) -> IUnknownRef? {
        switch iid {
            case __ABI_Windows_Storage_Streams.IInputStreamWrapper.IID:
                let wrapper = __ABI_Windows_Storage_Streams.IInputStreamWrapper(self)
                return wrapper!.queryInterface(iid)
            case __ABI_Windows_Foundation.IClosableWrapper.IID:
                let wrapper = __ABI_Windows_Foundation.IClosableWrapper(self)
                return wrapper!.queryInterface(iid)
            default: return nil
        }
    }
}
public typealias AnyIInputStream = any IInputStream

/// [Open Microsoft documentation](https://learn.microsoft.com/uwp/api/windows.storage.streams.iinputstreamreference)
public protocol IInputStreamReference : WinRTInterface {
    /// [Open Microsoft documentation](https://learn.microsoft.com/uwp/api/windows.storage.streams.iinputstreamreference.opensequentialreadasync)
    func openSequentialReadAsync() throws -> test_component.AnyIAsyncOperation<test_component.AnyIInputStream?>!
}

extension IInputStreamReference {
    public func queryInterface(_ iid: test_component.IID) -> IUnknownRef? {
        switch iid {
            case __ABI_Windows_Storage_Streams.IInputStreamReferenceWrapper.IID:
                let wrapper = __ABI_Windows_Storage_Streams.IInputStreamReferenceWrapper(self)
                return wrapper!.queryInterface(iid)
            default: return nil
        }
    }
}
public typealias AnyIInputStreamReference = any IInputStreamReference

/// [Open Microsoft documentation](https://learn.microsoft.com/uwp/api/windows.storage.streams.ioutputstream)
public protocol IOutputStream : test_component.IClosable {
    /// [Open Microsoft documentation](https://learn.microsoft.com/uwp/api/windows.storage.streams.ioutputstream.writeasync)
    func writeAsync(_ buffer: test_component.AnyIBuffer!) throws -> test_component.AnyIAsyncOperationWithProgress<UInt32, UInt32>!
    /// [Open Microsoft documentation](https://learn.microsoft.com/uwp/api/windows.storage.streams.ioutputstream.flushasync)
    func flushAsync() throws -> test_component.AnyIAsyncOperation<Bool>!
}

extension IOutputStream {
    public func queryInterface(_ iid: test_component.IID) -> IUnknownRef? {
        switch iid {
            case __ABI_Windows_Storage_Streams.IOutputStreamWrapper.IID:
                let wrapper = __ABI_Windows_Storage_Streams.IOutputStreamWrapper(self)
                return wrapper!.queryInterface(iid)
            case __ABI_Windows_Foundation.IClosableWrapper.IID:
                let wrapper = __ABI_Windows_Foundation.IClosableWrapper(self)
                return wrapper!.queryInterface(iid)
            default: return nil
        }
    }
}
public typealias AnyIOutputStream = any IOutputStream

/// [Open Microsoft documentation](https://learn.microsoft.com/uwp/api/windows.storage.streams.irandomaccessstream)
public protocol IRandomAccessStream : test_component.IClosable, IInputStream, IOutputStream {
    /// [Open Microsoft documentation](https://learn.microsoft.com/uwp/api/windows.storage.streams.irandomaccessstream.getinputstreamat)
    func getInputStreamAt(_ position: UInt64) throws -> test_component.AnyIInputStream!
    /// [Open Microsoft documentation](https://learn.microsoft.com/uwp/api/windows.storage.streams.irandomaccessstream.getoutputstreamat)
    func getOutputStreamAt(_ position: UInt64) throws -> test_component.AnyIOutputStream!
    /// [Open Microsoft documentation](https://learn.microsoft.com/uwp/api/windows.storage.streams.irandomaccessstream.seek)
    func seek(_ position: UInt64) throws
    /// [Open Microsoft documentation](https://learn.microsoft.com/uwp/api/windows.storage.streams.irandomaccessstream.clonestream)
    func cloneStream() throws -> test_component.AnyIRandomAccessStream!
    /// [Open Microsoft documentation](https://learn.microsoft.com/uwp/api/windows.storage.streams.irandomaccessstream.canread)
    var canRead: Bool { get }
    /// [Open Microsoft documentation](https://learn.microsoft.com/uwp/api/windows.storage.streams.irandomaccessstream.canwrite)
    var canWrite: Bool { get }
    /// [Open Microsoft documentation](https://learn.microsoft.com/uwp/api/windows.storage.streams.irandomaccessstream.position)
    var position: UInt64 { get }
    /// [Open Microsoft documentation](https://learn.microsoft.com/uwp/api/windows.storage.streams.irandomaccessstream.size)
    var size: UInt64 { get set }
}

extension IRandomAccessStream {
    public func queryInterface(_ iid: test_component.IID) -> IUnknownRef? {
        switch iid {
            case __ABI_Windows_Storage_Streams.IRandomAccessStreamWrapper.IID:
                let wrapper = __ABI_Windows_Storage_Streams.IRandomAccessStreamWrapper(self)
                return wrapper!.queryInterface(iid)
            case __ABI_Windows_Foundation.IClosableWrapper.IID:
                let wrapper = __ABI_Windows_Foundation.IClosableWrapper(self)
                return wrapper!.queryInterface(iid)
            case __ABI_Windows_Storage_Streams.IInputStreamWrapper.IID:
                let wrapper = __ABI_Windows_Storage_Streams.IInputStreamWrapper(self)
                return wrapper!.queryInterface(iid)
            case __ABI_Windows_Storage_Streams.IOutputStreamWrapper.IID:
                let wrapper = __ABI_Windows_Storage_Streams.IOutputStreamWrapper(self)
                return wrapper!.queryInterface(iid)
            default: return nil
        }
    }
}
public typealias AnyIRandomAccessStream = any IRandomAccessStream

/// [Open Microsoft documentation](https://learn.microsoft.com/uwp/api/windows.storage.streams.irandomaccessstreamreference)
public protocol IRandomAccessStreamReference : WinRTInterface {
    /// [Open Microsoft documentation](https://learn.microsoft.com/uwp/api/windows.storage.streams.irandomaccessstreamreference.openreadasync)
    func openReadAsync() throws -> test_component.AnyIAsyncOperation<test_component.AnyIRandomAccessStreamWithContentType?>!
}

extension IRandomAccessStreamReference {
    public func queryInterface(_ iid: test_component.IID) -> IUnknownRef? {
        switch iid {
            case __ABI_Windows_Storage_Streams.IRandomAccessStreamReferenceWrapper.IID:
                let wrapper = __ABI_Windows_Storage_Streams.IRandomAccessStreamReferenceWrapper(self)
                return wrapper!.queryInterface(iid)
            default: return nil
        }
    }
}
public typealias AnyIRandomAccessStreamReference = any IRandomAccessStreamReference

/// [Open Microsoft documentation](https://learn.microsoft.com/uwp/api/windows.storage.streams.irandomaccessstreamwithcontenttype)
public protocol IRandomAccessStreamWithContentType : test_component.IClosable, IInputStream, IOutputStream, IRandomAccessStream, IContentTypeProvider {
}

extension IRandomAccessStreamWithContentType {
    public func queryInterface(_ iid: test_component.IID) -> IUnknownRef? {
        switch iid {
            case __ABI_Windows_Storage_Streams.IRandomAccessStreamWithContentTypeWrapper.IID:
                let wrapper = __ABI_Windows_Storage_Streams.IRandomAccessStreamWithContentTypeWrapper(self)
                return wrapper!.queryInterface(iid)
            case __ABI_Windows_Foundation.IClosableWrapper.IID:
                let wrapper = __ABI_Windows_Foundation.IClosableWrapper(self)
                return wrapper!.queryInterface(iid)
            case __ABI_Windows_Storage_Streams.IInputStreamWrapper.IID:
                let wrapper = __ABI_Windows_Storage_Streams.IInputStreamWrapper(self)
                return wrapper!.queryInterface(iid)
            case __ABI_Windows_Storage_Streams.IOutputStreamWrapper.IID:
                let wrapper = __ABI_Windows_Storage_Streams.IOutputStreamWrapper(self)
                return wrapper!.queryInterface(iid)
            case __ABI_Windows_Storage_Streams.IRandomAccessStreamWrapper.IID:
                let wrapper = __ABI_Windows_Storage_Streams.IRandomAccessStreamWrapper(self)
                return wrapper!.queryInterface(iid)
            case __ABI_Windows_Storage_Streams.IContentTypeProviderWrapper.IID:
                let wrapper = __ABI_Windows_Storage_Streams.IContentTypeProviderWrapper(self)
                return wrapper!.queryInterface(iid)
            default: return nil
        }
    }
}
public typealias AnyIRandomAccessStreamWithContentType = any IRandomAccessStreamWithContentType

extension test_component.InputStreamOptions {
    public static var none : test_component.InputStreamOptions {
        __x_ABI_CWindows_CStorage_CStreams_CInputStreamOptions_None
    }
    public static var partial : test_component.InputStreamOptions {
        __x_ABI_CWindows_CStorage_CStreams_CInputStreamOptions_Partial
    }
    public static var readAhead : test_component.InputStreamOptions {
        __x_ABI_CWindows_CStorage_CStreams_CInputStreamOptions_ReadAhead
    }
}
extension test_component.InputStreamOptions: @retroactive Hashable, @retroactive Codable, @retroactive @unchecked Sendable {}

extension test_component.UnicodeEncoding {
    public static var utf8 : test_component.UnicodeEncoding {
        __x_ABI_CWindows_CStorage_CStreams_CUnicodeEncoding_Utf8
    }
    public static var utf16LE : test_component.UnicodeEncoding {
        __x_ABI_CWindows_CStorage_CStreams_CUnicodeEncoding_Utf16LE
    }
    public static var utf16BE : test_component.UnicodeEncoding {
        __x_ABI_CWindows_CStorage_CStreams_CUnicodeEncoding_Utf16BE
    }
}
extension test_component.UnicodeEncoding: @retroactive Hashable, @retroactive Codable, @retroactive @unchecked Sendable {}

