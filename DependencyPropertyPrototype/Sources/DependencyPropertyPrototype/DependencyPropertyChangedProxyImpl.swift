// implementation of the DependencyPropertyChangedProxy protocol. We require a separate implementation type
// because this type understands the ABI, whereas the generic DependencyPropertyChangedProxy just understands
// the value

struct DependencyPropertyChangedProxyImpl<Owner: DOAbi, Value>: DependencyPropertyChangedProxy {
    let property: DependencyPropertyHandle
    let owner: Owner

    init(property: DependencyPropertyHandle, owner: Owner, set: @escaping (Owner, Value) -> Void, get: @escaping (Owner) -> Value) {
        self.property = property
        self.owner = owner
        self.set = { value in
            set(owner, value)
        }
        self.get = {
            return get(owner)
        }
    }
    let set: (Value) -> Void
    let get: () -> Value

    func sink(_ handler: @escaping (Value, Value) -> Void) {
        owner.registerPropertyChanged(property) { (oldValue, newValue) in
            handler(oldValue as! Value, newValue as! Value)
        }
    }
}