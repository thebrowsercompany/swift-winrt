import Foundation

// Not strongly typed, we lose the type safety of the associatedtype anyways
// when we cast to `any MakeFromAbi`, plus that requires a lot more exported
// simples than we want
public protocol MakeFromAbi {
    static func from(abi: test_component.IInspectable) -> Any
}

func makeFrom(abi: test_component.IInspectable) -> Any? {
    // When creating a swift class which represents this type, we want to get the class name that we're trying to create
    // via GetRuntimeClassName so that we can create the proper derived type. For example, the API may return UIElement,
    // but we want to return a Button type.

    // Note that we'll *never* be trying to create an app implemented object at this point
    let className = try? abi.GetSwiftClassName()
    guard let className, let makerType = NSClassFromString("\(className)_MakeFromAbi") as? any MakeFromAbi.Type else {
        return nil
    }
    return makerType.from(abi: abi)
}