class SetterAbi : DOAbi{
    override init() {
        self.property = DependencyPropertyHandle()
        super.init()
    }
    private var property: DependencyPropertyHandle
    func setProperty(_ value: DependencyPropertyHandle) {
        let oldValue = property
        self.property = value
        handler?(oldValue, value)
    }
    func getProperty() -> DependencyPropertyHandle {
        return property
    }

    private var value: Any?
    func setValue(_ value: Any?) {
        //let oldValue = self.value
        self.value = value
        //handler?(oldValue, value)
    }
    func getValue() -> Any? {
        return value
    }
}