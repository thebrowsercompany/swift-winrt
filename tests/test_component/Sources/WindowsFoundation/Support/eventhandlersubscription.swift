import Foundation
import WinSDK
import CWinRT

struct EventHandlerSubscription<Handler> {
  var handler: Handler
  var token: CWinRT.EventRegistrationToken
}

struct EventHandlerSubscriptions<Handler> {
    typealias Subscription = EventHandlerSubscription<Handler>
    private var buffer = [Subscription]()
    let lock = NSLock()
    init(){}
    mutating func append(_ handler: Handler) -> CWinRT.EventRegistrationToken {
        let token = CWinRT.EventRegistrationToken(value: Int64(Int(bitPattern: Unmanaged.passUnretained(handler as AnyObject).toOpaque())))
        lock.lock()
        buffer.append(.init(handler: handler, token: token))
        lock.unlock()
        return token
    }
    
    mutating func remove(token: CWinRT.EventRegistrationToken) {
        lock.lock()
        // The semantics when the same event handler is added multiple times
        // is to append to the end and to remove the last occurrence first.
        if let index = buffer.lastIndex(where: { $0.token == token }) {
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
