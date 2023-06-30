import WinSDK
import Ctest_component

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

struct NoopDisposable : Disposable {
    func dispose() {}
}