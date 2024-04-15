import XCTest
import test_component
import Foundation

class CollectionTests : XCTestCase {
  
  public func testVector_asInput() throws {
    let array = ["Hello", "Goodbye", "Goodnight"]

    let result = try CollectionTester.inVector(array.toVector())
    XCTAssertEqual(result, "Hello")
  }

  public func testVector_asReturn() throws {
    let tester = CollectionTester()
    let vector = try tester.returnStoredStringVector()!
    XCTAssertEqual(vector.count, 1)
    XCTAssertEqual(vector[0], "Hello")
  }

  public func testVector_mutate() throws {
    let tester = CollectionTester()
    let vector = try tester.returnStoredStringVector()!
    XCTAssertEqual(vector.count, 1)

    vector.append("Goodbye")
    XCTAssertEqual(vector.count, 2)

    // Make sure the returned vector has the same data
    // as the one we modified
    let vector2 = try tester.returnStoredStringVector()!
    XCTAssertEqual(vector2.count, vector.count)
    XCTAssertEqual(vector2[0], vector2[0])
    XCTAssertEqual(vector2[1], vector2[1])
    XCTAssertEqual(vector2[1], "Goodbye")

    vector2.append("Goodnight")

    XCTAssertEqual(vector2.count, 3)
    XCTAssertEqual(vector.count, 3)
    XCTAssertEqual(vector2[2], vector2[2])
    XCTAssertEqual(vector2[2], "Goodnight")
  }

  public func testArrayVectorIsIterable() throws {
    XCTAssertNotNil(try CollectionTester.vectorAsIterable(["a", "b", "c"].toVector()))
  }

  public func testVectorObject_toCallback() throws {
    let person = Person(firstName: "John", lastName: "Doe", age: 42)
    let array:[Any?] = [person, "Goodbye", 1]

    try CollectionTester.getObjectAt(array.toVector(), 0) { 
        XCTAssertEqual($0 as! Person, person)
    }
  }

  public func testMap_asInput() throws {
    let dictionary = ["A": "Alpha"]
    let value = try CollectionTester.inMap(dictionary.toMap())
    XCTAssertEqual(value, "Alpha")
  }
  
  public func testMap_asReturn() throws {
    let tester = CollectionTester()
    let map = try tester.returnMapFromStringToString()!
    XCTAssertEqual(map.count, 1)
    XCTAssert(map.hasKey("A"))
    XCTAssertEqual(map.lookup("A"), "Alpha")
    XCTAssert(!map.hasKey("Z"))
  }

  public func testMap_mutate() throws {
    let tester = CollectionTester()
    let map = try tester.returnMapFromStringToString()!
    XCTAssert(map.hasKey("A"))
    XCTAssertEqual(map.lookup("A"), "Alpha")
    
    XCTAssert(map.insert("A", "Aleph")) // Returns true if replacing
    XCTAssert(map.hasKey("A"))
    XCTAssertEqual(map.lookup("A"), "Aleph")
    let value = try CollectionTester.inMap(map)
    XCTAssertEqual(value, "Aleph")
  }
}

var collectionTests: [XCTestCaseEntry] = [
  testCase([
    ("testMap_asInput", CollectionTests.testMap_asInput),
    ("testMap_asReturn", CollectionTests.testMap_asReturn),
    ("testMap_mutate", CollectionTests.testMap_mutate),
    ("testVector_asInput", CollectionTests.testVector_asInput),
    ("testVector_asReturn", CollectionTests.testVector_asReturn),
    ("testVector_mutate", CollectionTests.testVector_mutate),
    ("testArrayVectorIsIterable", CollectionTests.testArrayVectorIsIterable),
    ("testVectorObject_toCallback", CollectionTests.testVectorObject_toCallback),
  ])
]
