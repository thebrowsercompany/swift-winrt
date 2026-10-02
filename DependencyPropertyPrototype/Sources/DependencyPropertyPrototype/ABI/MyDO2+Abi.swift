class MyDO2ABI : MyDOABI{
    private var myBoolProperty:Bool = false
    func setMyBoolProperty(_ value: Bool) {
        let oldValue = myBoolProperty
        self.myBoolProperty = value
        handler?(oldValue, value)
    }
    func getMyBoolProperty() -> Bool {
        return myBoolProperty
    }

}