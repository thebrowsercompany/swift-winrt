import WinSDK
import C_BINDINGS_MODULE

public typealias EventHandler<Args> = (Any?, Args) -> ()
public typealias TypedEventHandler<Sender, Args> = (Sender, Args) -> ()

public protocol Disposable {
  func dispose()
}

public struct Event<Handler>
{
    fileprivate var add: (Handler) -> EventRegistrationToken
    fileprivate var remove: (_ token: EventRegistrationToken) -> Void
    
    public init(add: @escaping (Handler) -> EventRegistrationToken, remove: @escaping (_ token: EventRegistrationToken) -> Void) {
        self.add = add
        self.remove = remove
    }

    @discardableResult public func addHandler(_ handler: Handler) -> any Disposable {
        let token = add(handler)
        return EventCleanup(token: token, closeAction: remove)
    }

    public func removeHandler(_ token: EventRegistrationToken) {
        remove(token)
    }
}

public protocol DisposableWithToken : Disposable {
    var token: C_BINDINGS_MODULE.EventRegistrationToken { get }
}

struct EventCleanup: DisposableWithToken {
    var token: EventRegistrationToken
    var closeAction: (_ token: EventRegistrationToken) -> Void
    func dispose() {
        closeAction(token)
    }
}

public struct NoopDisposable : Disposable {
    public func dispose() {}
}