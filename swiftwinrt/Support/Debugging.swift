import WinSDK
import Foundation

// Use with debugPrint, dump, etc. to simultaneously output to the debugger and
// console window.
//
// debugPrint("Useful stuff: \(var)", to: &debugOut)
// dump(self, to: &debugOut, name: "myObjectName", indent: 2, maxDepth: 4)
public var debugOut = DebugOutputStream()

public class DebugOutputStream : TextOutputStream {
  public func write(_ text: String) {
    let sanitizedText = text.replacingOccurrences(of: "▿", with: "+")
    print(sanitizedText, terminator: "")
    OutputDebugStringA(sanitizedText)
  }
}

public func debugPrintAddress(_ description: String, _ object: AnyObject) {
  debugPrint(description, terminator: "", to: &debugOut)
  debugPrint(Unmanaged.passUnretained(object).toOpaque(), to: &debugOut)
}

open class TrackableObject {
  internal var uniqueId: UInt = 0
  
  public init() {
    TrackedObjects.add(self)
  }
  
  deinit {
    TrackedObjects.remove(self)
  }
}

internal class TrackedObject {
  private var object: TrackableObject
  internal var uniqueId: UInt
  private static var idGenerator: UInt = 6000

  public init(_ object: TrackableObject) {
    self.object = object
    self.uniqueId = TrackedObject.idGenerator
    object.uniqueId = self.uniqueId
    TrackedObject.idGenerator += 1
  }  
}

public class TrackedObjects {
  private static var objects: Array<TrackedObject> = Array()

  public static func add(_ object: TrackableObject) {
    objects.append(TrackedObject(object))
  }

  public static func remove(_ object: TrackableObject) {
    objects.removeAll(where: { $0.uniqueId == object.uniqueId })
  }

  public static func dumpAll() {
    objects.forEach { object in 
      print("\r\n------------------------------------------------------------------\r\n")
      OutputDebugStringA("\r\n------------------------------------------------------------------\r\n")
      dump(object, to: &debugOut, name: String(describing: object), indent: 2, maxDepth: 10) 
    }
  }
}
