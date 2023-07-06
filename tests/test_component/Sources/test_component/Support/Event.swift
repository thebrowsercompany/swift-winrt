import WinSDK
import Ctest_component

public typealias EventHandler<Args> = (Any?, Args) -> ()
public typealias TypedEventHandler<Sender, Args> = (Sender, Args) -> ()

public protocol Disposable {
  func dispose()
}

public struct Event<Handler> {
    fileprivate var add: (Handler) -> EventRegistrationToken
    fileprivate var remove: (_ token: EventRegistrationToken) -> Void
    
    public init(add: @escaping (Handler) -> EventRegistrationToken, remove: @escaping (_ token: EventRegistrationToken) -> Void) {
        self.add = add
        self.remove = remove
    }

    @discardableResult public func addHandler(_ handler: Handler) -> EventCleanup {
        let token = add(handler)
        return EventCleanup(token: token, closeAction: remove)
    }

    public func removeHandler(_ token: EventRegistrationToken) {
        remove(token)
    }
}

public struct EventCleanup: Disposable {
    fileprivate let token: EventRegistrationToken
    let closeAction: (_ token: EventRegistrationToken) -> Void
    public init(token: EventRegistrationToken, closeAction: @escaping (_ token: EventRegistrationToken) -> Void) {
        self.token = token
        self.closeAction = closeAction
    }

    public func dispose() {
        closeAction(token)
    }
}

extension Ctest_component.EventRegistrationToken {
  public static func from(swift: EventCleanup) -> Ctest_component.EventRegistrationToken {
    return swift.token
  }
}
