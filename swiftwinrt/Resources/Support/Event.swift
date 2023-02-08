import WinSDK
import C_BINDINGS_MODULE

public class Event<Data, Return> {
    open func addHandler(handler: @escaping (Data) -> Return) -> Disposable? { fatalError("not implemented") }
}

public class EventImpl<Delegate: WinRTDelegate>: Event<Delegate.Data, Delegate.Return> {
    var register: IEventRegistration
    weak var owner: IInspectable?
    public init(register: IEventRegistration, owner: SUPPORT_MODULE.IInspectable){
        self.register = register
        self.owner = owner
    }

    override open func addHandler(handler: @escaping (Delegate.Data) -> Delegate.Return) -> Disposable? {
        guard let owner = owner else { return nil }
        let delegate = Delegate(handler: handler)
        register.add(delegate: delegate , for: owner)
        return EventCleanup(delegate: delegate, event: register, impl: owner)
    }
}

class EventCleanup : Disposable {
    weak var delegate: (any WinRTDelegate)?
    weak var event: IEventRegistration?
    weak var impl: SUPPORT_MODULE.IInspectable?
    init(delegate: any WinRTDelegate, event: IEventRegistration, impl: IInspectable?)
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
    func add(delegate: any WinRTDelegate, for: IInspectable)
    func remove(delegate: any WinRTDelegate, for: IInspectable)
}

public protocol WinRTDelegate : IWinRTObject {
    associatedtype Data
    associatedtype Return
    init(handler: @escaping (Data) -> Return)
    var handler: (Data) -> Return { get }
    var token: C_BINDINGS_MODULE.EventRegistrationToken? { get set }

}

public protocol Disposable {
 func dispose()
}

public func += <Data, Return> (left: Event<Data, Return>, right:@escaping (Data) -> Return) -> Disposable? {
  left.addHandler(handler: right)
}