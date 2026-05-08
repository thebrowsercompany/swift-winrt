class MyDOABI : FrameworkElementABI{
    private var myProperty:String = ""
    func setMyProperty(_ value: String) {
        let oldValue = myProperty
        self.myProperty = value
        handler?(oldValue, value)
    }
    func getMyProperty() -> String {
        return myProperty
    }

    private var myIntProperty:Int = 0
    func setMyIntProperty(_ value: Int) {
        let oldValue = myIntProperty
        self.myIntProperty = value
        handler?(oldValue, value)
    }
    func getMyIntProperty() -> Int {
        return myIntProperty
    }


}