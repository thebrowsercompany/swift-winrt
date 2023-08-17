public protocol FutureValue<TResult>: WinRTInterface {
    associatedtype TResult
    func get() async throws -> TResult
}

public typealias AnyFutureValue<T> = any FutureValue<T>
