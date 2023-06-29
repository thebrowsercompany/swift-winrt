import Foundation
import WinSDK
import C_BINDINGS_MODULE

struct EventHandlerSubscription<Data, Return>
{
  typealias Handler = (Data) -> Return
  var handler: Handler
  var token: C_BINDINGS_MODULE.EventRegistrationToken
}

struct EventHandlerSubscriptions<Data, Return> {
    typealias Subscription = EventHandlerSubscription<Data, Return>
    private var buffer = [Subscription]()
    let lock = NSLock()
    init(){}
    mutating func append(_ handler: @escaping Subscription.Handler) -> C_BINDINGS_MODULE.EventRegistrationToken {
        let token = C_BINDINGS_MODULE.EventRegistrationToken(value: Int64(Int(bitPattern: Unmanaged.passUnretained(handler as AnyObject).toOpaque())))
        lock.lock()
        buffer.append(.init(handler: handler, token: token))
        lock.unlock()
        return token
    }
    
    mutating func remove(item: C_BINDINGS_MODULE.EventRegistrationToken) {
        lock.lock()
        buffer = buffer.filter { $0.token != item }
        lock.unlock()
    }

    func getInvocationList() -> [Subscription.Handler] {
      lock.lock()
      let result = buffer.map { $0.handler }
      lock.unlock()
      return result
    }
}