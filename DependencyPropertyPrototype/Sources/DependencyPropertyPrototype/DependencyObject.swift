// This prototype of DependencyObject is a "dummy" implementation that mimics how the swiftwinrt code gen
// works today. The public swift type wraps an internal type. The internal type is what talks to the actual
// ABI.

open class DependencyObject {

    public init(fromABi: DOAbi) {
    }

    func registerPropertyChanged(_ dp: DependencyPropertyHandle, _ handler: (Any?, Any?) -> Void) {
        print("registerPropertyChanged")
    }

    //@Styled var style: StyleBase?
}
