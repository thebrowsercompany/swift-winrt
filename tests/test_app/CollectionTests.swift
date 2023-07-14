import XCTest
import test_component
import Foundation

class CollectionTests : XCTestCase {
  
  public func testVector() throws {
    let array = ["Hello", "Goodbye", "Goodnight"]

    var result = CollectionTester.inVector(array.toVector())
    print(result)
    XCTAssertEqual(result, "Hello")

    let classy = CollectionTester()
    let vector = try classy.returnStoredStringVector()!
    XCTAssertEqual(vector.count, 1)
    print(vector[0])

    result = CollectionTester.inVector(vector)

    print(result)
    XCTAssertEqual(result, vector[0])

    vector.append("Goodbye")
    XCTAssertEqual(vector.count, 2)
    // Make sure the returned vector has the same data
    // as the one we modified
    let vector2 = try classy.returnStoredStringVector()!
    XCTAssertEqual(vector2.count, vector.count)
    XCTAssertEqual(vector2[0], vector2[0])
    XCTAssertEqual(vector2[1], vector2[1])

    vector2.append("Goodnight")

    XCTAssertEqual(vector2.count, 3)
    XCTAssertEqual(vector.count, 3)
    XCTAssertEqual(vector2[2], vector2[2])
  }

  public func testMap_asInput() {
    let dictionary = ["A": "Alpha"]
    let value = CollectionTester.inMap(dictionary.toMap())
    XCTAssertEqual(value, "Alpha")
  }
  
  public func testMap_asReturn() throws {
    let classy = CollectionTester()
    let map = try classy.returnMapFromStringToString()!
    XCTAssertEqual(map.count, 1)
    XCTAssert(map.hasKey("A"))
    XCTAssertEqual(map.lookup("A"), "Alpha")
    XCTAssert(!map.hasKey("Z"))
  }

  public func testMap_mutate() throws {
    let classy = CollectionTester()
    let map = try classy.returnMapFromStringToString()!
    XCTAssert(map.hasKey("A"))
    XCTAssertEqual(map.lookup("A"), "Alpha")
    
    XCTAssert(map.insert("A", "Aleph")) // Returns true if replacing
    XCTAssert(map.hasKey("A"))
    XCTAssertEqual(map.lookup("A"), "Aleph")
    let value = CollectionTester.inMap(map)
    XCTAssertEqual(value, "Aleph")
  }
}

var collectionTests: [XCTestCaseEntry] = [
  testCase([
    ("testMap_asInput", CollectionTests.testMap_asInput),
    ("testMap_asReturn", CollectionTests.testMap_asReturn),
    ("testMap_mutate", CollectionTests.testMap_mutate),
    ("testVector", CollectionTests.testVector),
  ])
]

