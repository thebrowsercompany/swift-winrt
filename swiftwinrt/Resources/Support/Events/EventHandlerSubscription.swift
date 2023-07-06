import Foundation
import WinSDK
import C_BINDINGS_MODULE

struct EventHandlerSubscription<Handler>
{
  var handler: Handler
  var token: C_BINDINGS_MODULE.EventRegistrationToken
}

struct EventHandlerSubscriptions<Handler> {
    typealias Subscription = EventHandlerSubscription<Handler>
    private var buffer = [Subscription]()
    let lock = NSLock()
    init(){}
    mutating func append(_ handler: Handler) -> C_BINDINGS_MODULE.EventRegistrationToken {
        let token = C_BINDINGS_MODULE.EventRegistrationToken(value: Int64(Int(bitPattern: Unmanaged.passUnretained(handler as AnyObject).toOpaque())))
        lock.lock()
        buffer.append(.init(handler: handler, token: token))
        lock.unlock()
        return token
    }
    
    mutating func remove(token: C_BINDINGS_MODULE.EventRegistrationToken) {
        lock.lock()
        if let index = buffer.firstIndex(where: { $0.token == token }) {
            buffer.remove(at: index)
        }
        lock.unlock()
    }

    func getInvocationList() -> [Handler] {
      lock.lock()
      let result = buffer.map { $0.handler }
      lock.unlock()
      return result
    }
}
