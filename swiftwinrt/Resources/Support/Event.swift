import WinSDK
import C_BINDINGS_MODULE

public class Event<Data, Return> {
    @discardableResult open func addHandler(_ handler: @escaping (Data) -> Return) -> Disposable? { fatalError("not implemented") }
}

public protocol RemoveHandler {
    func removeHandler(_ token: C_BINDINGS_MODULE.EventRegistrationToken)
}

/// EventSource is the class which implements handling event subscriptions, removals,
/// and invoking events for authoring events in Swift
public final class EventSource<Data, Return>: Event<Data, Return>, RemoveHandler
{
  public override init(){}

  @discardableResult public func invoke(_ args: Data) -> Return? {
    // Sort of weird behavior here, it would be *extremely* rare and bad practice
    // for delegates to have a return value for this reason. Nonetheless, it is
    // doable and this is the behavior of C#. If multiple handlers return a value
    // then the last one is returned. If callers want to get all the handlers and
    // invoke them to get the results, then that can be done
    var result: Return?
    for handler in handlers {
      result = handler(args)
    }
    return result
  }
  public typealias Handler = (Data) -> Return

  public var handlers: [Handler] {
    return Array(handlerTable.values)
  }

  override public func addHandler(_ handler: @escaping (Data) -> Return) -> Disposable? {
    let token = EventRegistrationToken(value: Int64(Int(bitPattern: Unmanaged.passUnretained(handler as AnyObject).toOpaque())))
    handlerTable[token] = handler
    return EventSourceCleanup(token: token, event: self)
  }

  public func removeHandler(_ token: EventRegistrationToken){
    self.handlerTable.removeValue(forKey: token)
  }

  private lazy var handlerTable = [EventRegistrationToken: Handler]()
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

public class EventImpl<Register: IEventRegistration>: Event<Register.Delegate.Data, Register.Delegate.Return> {
    var register: Register
    weak var owner: Register.Owner?
    public init(register: Register, owner: Register.Owner){
        self.register = register
        self.owner = owner
    }

    @discardableResult override public func addHandler(_ handler: @escaping (Register.Delegate.Data) -> Register.Delegate.Return) -> Disposable? {
        guard let owner = owner else { return nil }
        let token = register.add(handler: handler , for: owner)
        return EventCleanup(token: token, event: register, impl: owner)
    }
}

public protocol DisposableWithToken : Disposable {
    var token: C_BINDINGS_MODULE.EventRegistrationToken { get }
}

class EventCleanup<Register: IEventRegistration> : DisposableWithToken {
    private (set) public var token: C_BINDINGS_MODULE.EventRegistrationToken
    weak var event: Register?
    weak var impl: Register.Owner?
    init(token: C_BINDINGS_MODULE.EventRegistrationToken, event: Register, impl: Register.Owner?) {
        self.token = token
        self.event = event
        self.impl = impl
    }

    func dispose() {
        if let event = event,
           let impl = impl {
            event.remove(token: token, for: impl)
        }
    }
}

public protocol IEventRegistration<Delegate, Owner> : AnyObject {
    associatedtype Delegate: WinRTDelegate
    associatedtype Owner : AnyObject

    func add(handler: @escaping (Delegate.Data) -> Delegate.Return, for: Owner) -> C_BINDINGS_MODULE.EventRegistrationToken
    func remove(token: C_BINDINGS_MODULE.EventRegistrationToken, for: Owner)
}
public protocol WinRTDelegate : AnyObject {
    associatedtype Data
    associatedtype Return
    init(handler: @escaping (Data) -> Return)
    var handler: (Data) -> Return { get }
}

public protocol WinRTDelegateBridge<Data, Return>: AbiInterfaceImpl, WinRTDelegate where SwiftProjection == (Data) -> Return {
}

// The WinRTDelegateBridge doesn't actually hold a pointer to the SwiftABI, 
// rather the handler it creates and gives to the Swift holder keeps a strong
// reference to the SwiftABI object, which keeps the handler alive. 
public extension WinRTDelegateBridge {
    var _default: SwiftABI { fatalError("_default should not be accessed on a WinRTDelegateBridge") }
}

public typealias AnyWinRTDelegate = any WinRTDelegate
public protocol Disposable {
 func dispose()
}