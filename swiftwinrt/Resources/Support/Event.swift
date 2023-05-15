import WinSDK
import C_BINDINGS_MODULE

public class Event<Data, Return> {
    @discardableResult open func addHandler(_ handler: @escaping (Data) -> Return) -> Disposable? { fatalError("not implemented") }
}

public class EventImpl<Delegate: WinRTDelegate>: Event<Delegate.Data, Delegate.Return> {
    var register: IEventRegistration
    weak var owner: IInspectable?
    public init(register: IEventRegistration, owner: SUPPORT_MODULE.IInspectable){
        self.register = register
        self.owner = owner
    }

    @discardableResult override open func addHandler(_ handler: @escaping (Delegate.Data) -> Delegate.Return) -> Disposable? {
        guard let owner = owner else { return nil }
        let delegate = Delegate(handler: handler)
        register.add(delegate: delegate , for: owner)
        return EventCleanup(delegate: delegate, event: register, impl: owner)
    }
}

class EventCleanup : Disposable {
    weak var delegate: AnyWinRTDelegate?
    weak var event: IEventRegistration?
    weak var impl: SUPPORT_MODULE.IInspectable?
    init(delegate: AnyWinRTDelegate, event: IEventRegistration, impl: IInspectable?)
    {
        self.delegate = delegate
        self.event = event
        self.impl = impl
    }

    func dispose() {
        if let delegate = delegate,
           let event = event,
           let impl = impl {
            event.remove(delegate: delegate, for: impl)
        }
    }
}

public protocol IEventRegistration : AnyObject {
    func add(delegate: AnyWinRTDelegate, for: IInspectable)
    func remove(delegate: AnyWinRTDelegate, for: IInspectable)
}

public protocol WinRTDelegate : IWinRTObject {
    associatedtype Data
    associatedtype Return
    init(handler: @escaping (Data) -> Return)
    var handler: (Data) -> Return { get }
    var token: C_BINDINGS_MODULE.EventRegistrationToken? { get set }

}

public typealias AnyWinRTDelegate = any WinRTDelegate
public protocol Disposable {
 func dispose()
}