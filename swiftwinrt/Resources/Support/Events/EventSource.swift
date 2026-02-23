import Foundation
import WinSDK
import CWinRT

/// EventSource is the class which implements handling event subscriptions, removals,
/// and invoking events for authoring events in Swift
@propertyWrapper public class EventSource<Handler> {
    private var event: Event<Handler>!
    private var handlers = EventHandlerSubscriptions<Handler>()

    public init() {
        event = .init(
          add: { [weak self] in self?.handlers.append($0) ?? .init() },
          remove: { [weak self] in self?.handlers.remove(token: $0) }
        )
    }

    public var wrappedValue: Event<Handler> { event }

    public func getInvocationList() -> [Handler] {
      handlers.getInvocationList()
    }
}

extension CWinRT.EventRegistrationToken: @retroactive Hashable {
  public static func == (lhs: CWinRT.EventRegistrationToken, rhs: CWinRT.EventRegistrationToken) -> Bool {
    return lhs.value == rhs.value
  }

   public func hash(into hasher: inout Hasher) {
        hasher.combine(value)
    }
}
