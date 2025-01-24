import Foundation

// Not strongly typed, we lose the type safety of the associatedtype anyways
// when we cast to `any MakeFromAbi`, plus that requires a lot more exported
// simples than we want
public protocol MakeFromAbi {
    associatedtype SwiftType
    static func from(abi: SUPPORT_MODULE.IInspectable) -> SwiftType
}

func make(typeName: SwiftTypeName, from abi: SUPPORT_MODULE.IInspectable) -> Any? {
    guard let makerType = NSClassFromString("\(typeName.module).\(typeName.typeName)Maker") as? any MakeFromAbi.Type else {
        return nil
    }
    return makerType.from(abi: abi)
}

func makeFrom(abi: SUPPORT_MODULE.IInspectable) -> Any? {
    // When creating a swift class which represents this type, we want to get the class name that we're trying to create
    // via GetRuntimeClassName so that we can create the proper derived type. For example, the API may return UIElement,
    // but we want to return a Button type.

    // Note that we'll *never* be trying to create an app implemented object at this point
    guard let className = try? abi.GetSwiftTypeName() else { return nil }

    return make(typeName: className, from: abi)
}

func make<T:AnyObject>(type: T.Type, from abi: SUPPORT_MODULE.IInspectable) -> T? {
    let classString = NSStringFromClass(type).split(separator: ".", maxSplits: 2)
    return make(typeName: SwiftTypeName(module: String(classString[0]), typeName: String(classString[1])), from: abi) as? T
}
