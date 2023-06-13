import WinSDK
import Ctest_component

public class Event<Data, Return> {
    @discardableResult open func addHandler(_ handler: @escaping (Data) -> Return) -> Disposable? { fatalError("not implemented") }
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

class EventCleanup<Register: IEventRegistration> : Disposable {
    var token: Ctest_component.EventRegistrationToken
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
    associatedtype Owner : IInspectable

    func add(handler: @escaping (Delegate.Data) -> Delegate.Return, for: Owner) -> Ctest_component.EventRegistrationToken
    func remove(token: Ctest_component.EventRegistrationToken, for: Owner)
}
public protocol WinRTDelegate : AnyObject {
    associatedtype Data
    associatedtype Return
    init(handler: @escaping (Data) -> Return)
    var handler: (Data) -> Return { get }
}

public protocol WinRTDelegateBridge<Data, Return>: AbiBridge, WinRTDelegate where SwiftProjection == (Data) -> Return {
}

public typealias AnyWinRTDelegate = any WinRTDelegate
public protocol Disposable {
 func dispose()
}