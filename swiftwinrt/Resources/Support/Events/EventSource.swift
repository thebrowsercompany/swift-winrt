import Foundation
import WinSDK
import C_BINDINGS_MODULE

public protocol RemoveHandler {
    func removeHandler(_ token: C_BINDINGS_MODULE.EventRegistrationToken)
}

/// EventSource is the class which implements handling event subscriptions, removals,
/// and invoking events for authoring events in Swift
public final class EventSource<Data, Return>: Event<Data, Return>, RemoveHandler
{
  public typealias Handler = (Data) -> Return

  public override init(){}

  @discardableResult public func invoke(_ args: Data) -> Return? {
    // Sort of weird behavior here, it would be *extremely* rare and bad practice
    // for delegates to have a return value for this reason. Nonetheless, it is
    // doable and this is the behavior of C#. If multiple handlers return a value
    // then the last one is returned. If callers want to get all the handlers and
    // invoke them to get the results, then that can be done
    var result: Return?
    for handler in getInvocationList() {
      result = handler(args)
    }
    return result
  }

  public func getInvocationList() -> [Handler] {
    handlers.getInvocationList()
  }

  override public func addHandler(_ handler: @escaping (Data) -> Return) -> Disposable? {
    let token = handlers.append(handler)
    return EventSourceCleanup(token: token, event: self)
  }

  public func removeHandler(_ token: EventRegistrationToken){
    handlers.remove(item: token)
  }

  private var handlers = EventHandlerSubscriptions<Data, Return>()
}

extension C_BINDINGS_MODULE.EventRegistrationToken: Hashable {
  public static func == (lhs: C_BINDINGS_MODULE.EventRegistrationToken, rhs: C_BINDINGS_MODULE.EventRegistrationToken) -> Bool {
    return lhs.value == rhs.value
  }

   public func hash(into hasher: inout Hasher) {
        hasher.combine(value)
    }
}

extension C_BINDINGS_MODULE.EventRegistrationToken {
  public static func from(swift: DisposableWithToken) -> C_BINDINGS_MODULE.EventRegistrationToken {
    return swift.token
  }
}

public protocol DisposableWithToken : Disposable {
    var token: C_BINDINGS_MODULE.EventRegistrationToken { get }
}


class EventSourceCleanup<Data, Return> : DisposableWithToken {
    private (set) public var token: C_BINDINGS_MODULE.EventRegistrationToken
    weak var event: EventSource<Data, Return>?
    init(token: C_BINDINGS_MODULE.EventRegistrationToken, event: EventSource<Data, Return>) {
        self.token = token
        self.event = event
    }

    func dispose() {
      guard let event else { return }
      event.removeHandler(token)
    }
}
