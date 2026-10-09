# Dependency Property Prototype

This project contains a prototype to show how we can model DependencyProperites after `Published` properties.

Furthermore, we can also specially generate them in a manner which enforces type safety when using Styles/Setters.

See the code in `Sources` dir for the API and explanation.

See the Tests for use cases

## Usage

### Change notifications

```
let myDO = MyDO()
myDO.myProperty = "hello"
myDO.$myProperty.sink {
    XCTAssertEqual($0, "hello")
    XCTAssertEqual($1, "world")
}

myDO.myProperty = "world"
```

### Style type enforcements

```
let style = Style(targetType: MyDO.self) {
    Setter(.myProperty, "hi")
    Setter(.myIntProperty, 2)             
}

```

## Open Questions

The `Style` class has a type-erased base class `StyleBase` which is what the API would expose, however this type wouldn't be constructible. 

In theory, someone could still apply an incorrect `Style` to a framework element. For example, something like this is allowed:

```
let obj = MyDO()
obj.Style = Style(targetType.MyOtherDO) { ... }
```

Is there a way to enforce this? Should `Style` have an API called `applyTo` which takes the object it wants to be applied to?