class FrameworkElementABI: DOAbi {
    private var styleProperty: StyleBase?
    func setStyleProperty(_ value: StyleBase?) {
        let oldValue = styleProperty
        self.styleProperty = value
        handler?(oldValue, value)
    }
    func getStyleProperty() -> StyleBase? {
        return styleProperty
    }
}