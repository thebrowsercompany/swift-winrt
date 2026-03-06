import WinSDK
import CWinRT

/// EventSource is the class which implements handling event subscriptions, removals,
/// and invoking events for authoring events in Swift
@propertyWrapper public class EventSource<Handler> {
    private struct Subscription {
        var handler: Handler
        var token: CWinRT.EventRegistrationToken
    }

    private var event: Event<Handler>!
    private var subscriptions = [Subscription]()
    private var nextToken: Int64 = 0
    private var lock = SRWLock()

    public init() {
        event = .init(
          add: { [weak self] in self?.append($0) ?? .init() },
          remove: { [weak self] in self?.remove(token: $0) }
        )
    }

    public var wrappedValue: Event<Handler> { event }

    public func invokeAll(_ body: (Handler) throws -> Void) rethrows {
        let subscriptionsCopy = lock.withLock(.shared) { subscriptions }
        for subscription in subscriptionsCopy {
            try body(subscription.handler)
        }
    }

    private func append(_ handler: Handler) -> CWinRT.EventRegistrationToken {
        return lock.withLock(.exclusive) {
            let token = CWinRT.EventRegistrationToken(value: nextToken)
            defer { nextToken += 1 }
            subscriptions.append(.init(handler: handler, token: token))
            return token
        }
    }

    private func remove(token: CWinRT.EventRegistrationToken) {
        lock.withLock(.exclusive) {
            // The semantics when the same event handler is added multiple times
            // is to append to the end and to remove the last occurrence first.
            if let index = subscriptions.lastIndex(where: { $0.token == token }) {
                subscriptions.remove(at: index)
            }
        }
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
