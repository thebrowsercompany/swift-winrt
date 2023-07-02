import Foundation
import WinSDK
import C_BINDINGS_MODULE

/// EventSource is the class which implements handling event subscriptions, removals,
/// and invoking events for authoring events in Swift
@propertyWrapper public class EventSource<Handler>
{
    private var event: Event<Handler>!

    public init() {
        event = .init(
          add: { self.handlers.append($0) },
          remove: { self.handlers.remove(token: $0) }
        )
    }
    
    public var wrappedValue: Event<Handler> {
        get {
            return event
        }
    }
    
    public func getInvocationList() -> [Handler] {
      handlers.getInvocationList()
    }

    private var handlers = EventHandlerSubscriptions<Handler>()
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
