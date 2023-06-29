import WinSDK
import Ctest_component

public class Event<Data, Return> {
    @discardableResult open func addHandler(_ handler: @escaping (Data) -> Return) -> Disposable? { fatalError("not implemented") }
}

/// EventSource is the class which implements handling event subscriptions, removals,
/// and invoking events for authoring events in Swift
public final class EventSource<Data, Return>: Event<Data, Return>
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

extension Ctest_component.EventRegistrationToken: Hashable {
  public static func == (lhs: Ctest_component.EventRegistrationToken, rhs: Ctest_component.EventRegistrationToken) -> Bool {
    return lhs.value == rhs.value
  }

   public func hash(into hasher: inout Hasher) {
        hasher.combine(value)
    }
}

class EventSourceCleanup<Data, Return> : DisposableWithToken {
    private (set) public var token: Ctest_component.EventRegistrationToken
    weak var event: EventSource<Data, Return>?
    init(token: Ctest_component.EventRegistrationToken, event: EventSource<Data, Return>) {
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

protocol DisposableWithToken : Disposable {
    var token: Ctest_component.EventRegistrationToken { get }
}

class EventCleanup<Register: IEventRegistration> : DisposableWithToken {
    private (set) public var token: Ctest_component.EventRegistrationToken
    weak var event: Register?
    weak var impl: Register.Owner?
    init(token: Ctest_component.EventRegistrationToken, event: Register, impl: Register.Owner?) {
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

    func add(handler: @escaping (Delegate.Data) -> Delegate.Return, for: Owner) -> Ctest_component.EventRegistrationToken
    func remove(token: Ctest_component.EventRegistrationToken, for: Owner)
}
public protocol WinRTDelegate : AnyObject {
    associatedtype Data
    associatedtype Return
    init(handler: @escaping (Data) -> Return)
    var handler: (Data) -> Return { get }
}

public protocol EventHandler<Args> : WinRTDelegate where Data == (Any, Args), Return == Void {
  associatedtype Args
}

public protocol TypedEventHandler<Sender, Args> : WinRTDelegate where Data == (Sender, Args), Return == Void {
  associatedtype Sender
  associatedtype Args
}

public protocol WinRTDelegateBridge<Data, Return>: AbiBridge, WinRTDelegate where SwiftProjection == (Data) -> Return {
}

public protocol WinRTDelegateImpl<Data, Return> : WinRTDelegateBridge, AbiInterfaceImpl {
}


public typealias AnyWinRTDelegate = any WinRTDelegate
public protocol Disposable {
 func dispose()
}