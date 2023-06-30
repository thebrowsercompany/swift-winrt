import WinSDK
import C_BINDINGS_MODULE

public class EventImpl<Register: IEventRegistration>: Event<Register.Delegate.Data, Register.Delegate.Return> {
    var register: Register
    weak var owner: Register.Owner?
    public init(register: Register, owner: Register.Owner){
        self.register = register
        self.owner = owner
    }

    @discardableResult override public func addHandler(_ handler: @escaping (Register.Delegate.Data) -> Register.Delegate.Return) -> Disposable {
        guard let owner = owner else { return NoopDisposable() }
        let token = register.add(handler: handler , for: owner)
        return EventCleanup(token: token, event: register, impl: owner)
    }
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

struct NoopDisposable : Disposable {
    func dispose() {}
}