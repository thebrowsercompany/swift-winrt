import XCTest
@testable import DependencyPropertyPrototype

final class DependencyPropertyPrototypeTests: XCTestCase {
    func testChangeNotification() throws {
        let myDO = MyDO()
        myDO.myProperty = "hello"
        myDO.$myProperty.sink {
            XCTAssertEqual($0, "hello")
            XCTAssertEqual($1, "world")
        }

        myDO.myProperty = "world"
    }

    func testStyleAndSettersConstrainedToType() throws {
        let style = Style(targetType: MyDO.self) {
            Setter(.myProperty, "hi")
            Setter(.myIntProperty, 2)             
        }

        /*
        Commented out style to show that the compiler will properly enforce that the type of the setter matches the type of the property
        let style2 = Style(targetType: MyDO.self) {
            Setter(.myOtherIntProperty, 3)        
        }
        The above code produces the following error:
        
        error: cannot convert value of type 'Setter<MyOtherDO>' to expected argument type 'Setter<MyDO>'
            Setter(.myOtherIntProperty, 3)
            ^
        note: arguments to generic parameter 'AppliedTo' ('MyOtherDO' and 'MyDO') are expected to be equal
        */

        let myDO = MyDO()
        myDO.style = Style(targetType: MyDO2.self)
    }
}
