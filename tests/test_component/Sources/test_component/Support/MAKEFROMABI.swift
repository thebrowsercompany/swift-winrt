import Foundation

// Not strongly typed, we lose the type safety of the associatedtype anyways
// when we cast to `any MakeFromAbi`, plus that requires a lot more exported
// simples than we want
public protocol MakeFromAbi {
    static func from(typeName: String, abi: test_component.IInspectable) -> Any?
}

func make(typeName: SwiftTypeName, from abi: test_component.IInspectable) -> Any? {
    print("making: \(typeName)")
    guard let makerType = NSClassFromString("\(typeName.module).__MakeFromAbi") as? any MakeFromAbi.Type else {
        print("failed to find maker type")
        return nil
    }
    return makerType.from(typeName: typeName.typeName, abi: abi)
}

func makeFrom(abi: test_component.IInspectable) -> Any? {
    // When creating a swift class which represents this type, we want to get the class name that we're trying to create
    // via GetRuntimeClassName so that we can create the proper derived type. For example, the API may return UIElement,
    // but we want to return a Button type.

    // Note that we'll *never* be trying to create an app implemented object at this point
    let className = try? abi.GetSwiftTypeName()
    guard let className else {
        return nil
    }
    return make(typeName: className, from: abi)
}

func make<T:AnyObject>(type: T.Type, from abi: test_component.IInspectable) -> T? {
    let classString = NSStringFromClass(type).split(separator: ".", maxSplits: 2)
    return make(typeName: SwiftTypeName(module: String(classString[0]), typeName: String(classString[1])), from: abi) as? T
}
