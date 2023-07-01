import Foundation
import WinSDK
import Ctest_component

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

extension Ctest_component.EventRegistrationToken: Hashable {
  public static func == (lhs: Ctest_component.EventRegistrationToken, rhs: Ctest_component.EventRegistrationToken) -> Bool {
    return lhs.value == rhs.value
  }

   public func hash(into hasher: inout Hasher) {
        hasher.combine(value)
    }
}

extension Ctest_component.EventRegistrationToken {
  public static func from(swift: DisposableWithToken) -> Ctest_component.EventRegistrationToken {
    return swift.token
  }
}
