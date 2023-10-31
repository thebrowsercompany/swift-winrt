import Foundation
import WinSDK
import C_BINDINGS_MODULE

/// EventSource is the class which implements handling event subscriptions, removals,
/// and invoking events for authoring events in Swift
@propertyWrapper public class EventSource<Handler> {
    private var event: Event<Handler>!
    private var handlers = EventHandlerSubscriptions<Handler>()

    public init() {
        event = .init(
          add: { self.handlers.append($0) },
          remove: { self.handlers.remove(token: $0) }
        )
    }

    public var wrappedValue: Event<Handler> { event }

    public func getInvocationList() -> [Handler] {
      handlers.getInvocationList()
    }
}

extension C_BINDINGS_MODULE.EventRegistrationToken: @retroactive Hashable {
  public static func == (lhs: C_BINDINGS_MODULE.EventRegistrationToken, rhs: C_BINDINGS_MODULE.EventRegistrationToken) -> Bool {
    return lhs.value == rhs.value
  }

   public func hash(into hasher: inout Hasher) {
        hasher.combine(value)
    }
}
