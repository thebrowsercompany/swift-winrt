// "Base" class which is type-erased and is the type that would be used at the API surface. It can't be constructed directly.
public class StyleBase: DependencyObject {
    fileprivate init(fromABI: StyleAbi){
        super.init(fromABi: fromABI)
    }
}

// The Style class would be specially handgenerated to be applied to a specific type.
public class Style<AppliedTo: FrameworkElement>: StyleBase {
    var targetType: AppliedTo.Type?
    var setters: [Setter<AppliedTo>] = []

    @resultBuilder
    public enum SetterBuilder {
        public static func buildBlock(_ setters: Setter<AppliedTo>...) -> [Setter<AppliedTo>] {
            setters
        }
    }

    public convenience init(targetType: AppliedTo.Type, @SetterBuilder _ builder: () -> [Setter<AppliedTo>]) {
        self.init(fromABI: StyleAbi())
        self.targetType = targetType
    }

    public convenience init(targetType: AppliedTo.Type) {
        self.init(fromABI: StyleAbi())
        self.targetType = targetType
    }
}

public class Style2<AppliedTo: FrameworkElement, T> : Style<AppliedTo> {

}

@propertyWrapper
public struct StyleContainer<EnclosingType:FrameworkElement, Value> {
    public var wrappedValue: Style2<EnclosingType, Value>?

    public init(wrappedValue: Style2<EnclosingType, Value>?) {
        self.wrappedValue = wrappedValue
    }
}

public protocol StyleContainerType where Self: FrameworkElement{
    typealias Styled<Value> = StyleContainer<Self, Value>
}

extension FrameworkElement: StyleContainerType {
}

