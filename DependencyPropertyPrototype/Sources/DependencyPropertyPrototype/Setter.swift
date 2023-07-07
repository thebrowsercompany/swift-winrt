// Like Style, Setters are going to be specially hand generated to apply to a specific type. Note
// that Setters don't partake in the DependencyProperties type sytem. It's not that they couldn't,
// but there is really no need for them to.
public class Setter<AppliedTo:FrameworkElement> : DependencyObject {
    public convenience init<Value>(_ property: DependencyProperties<AppliedTo, Value>, _ value: Value) {
        self.init(fromABi: SetterAbi())
        self.property = property.handle
        self.value = value
    }

    override init(fromABi: DOAbi) {
        super.init(fromABi: fromABi)
    }
    var property: DependencyPropertyHandle?
    var value: Any?
}
