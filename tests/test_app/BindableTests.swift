import WinSDK
import XCTest
import test_component
import Foundation

fileprivate class Person {
  var firstName: String
  var lastName: String
  init(firstName: String, lastName: String) {
    self.firstName = firstName
    self.lastName = lastName
  }
}

class BindableTests : XCTestCase {
  public func testAddItem() throws {
    let people = BindableVector<Person>()
    
    let listView = ListView()
    listView.itemsSource = people
    listView.onItemAdded.addHandler { (_, args) in
      let person = args as! Person
      XCTAssertEqual(person.firstName, "John")
      XCTAssertEqual(person.lastName, "Doe")
    }
    people.append(Person(firstName: "John", lastName: "Doe"))
  }
}

var bindableTests: [XCTestCaseEntry] = [
  testCase([
    ("AddItem", BindableTests.testAddItem),
  ])
]